#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ourcontract.h"

/* entry of call stack */
typedef struct _frame {
    const char* name;
    FILE* out_fp;
    int state_fd;
    int depth;
    struct _frame* parent;
} frame;

/* call stack */
static frame* curr_frame = NULL;
FILE* in;
FILE* out;

static inline bool processmantissadigit(char ch, int64_t* mantissa, int* mantissa_tzeros)
{
    if (ch == '0')
        *mantissa_tzeros = *mantissa_tzeros + 1;
    else {
        for (int i = 0; i <= (*mantissa_tzeros); ++i) {
            if ((*mantissa) > (UPPER_BOUND / 10LL))
                return false; /* overflow */
            *mantissa = *mantissa * 10;
        }
        *mantissa = *mantissa + ch - '0';
        *mantissa_tzeros = 0;
    }
    return true;
}

bool parsefixedpoint(const char* val, int decimals, int64_t* amount_out)
{
    int64_t mantissa = 0;
    int64_t exponent = 0;
    int mantissa_tzeros = 0;
    bool mantissa_sign = false;
    bool exponent_sign = false;
    int ptr = 0;
    int end = strlen(val);
    int point_ofs = 0;

    if (ptr < end && val[ptr] == '-') {
        mantissa_sign = true;
        ++ptr;
    }
    if (ptr < end) {
        if (val[ptr] == '0') {
            /* pass single 0 */
            ++ptr;
        } else if (val[ptr] >= '1' && val[ptr] <= '9') {
            while (ptr < end && val[ptr] >= '0' && val[ptr] <= '9') {
                if (!processmantissadigit(val[ptr], &mantissa, &mantissa_tzeros))
                    return false; /* overflow */
                ++ptr;
            }
        } else
            return false; /* missing expected digit */
    } else
        return false; /* empty string or loose '-' */
    if (ptr < end && val[ptr] == '.') {
        ++ptr;
        if (ptr < end && val[ptr] >= '0' && val[ptr] <= '9') {
            while (ptr < end && val[ptr] >= '0' && val[ptr] <= '9') {
                if (!processmantissadigit(val[ptr], &mantissa, &mantissa_tzeros))
                    return false; /* overflow */
                ++ptr;
                ++point_ofs;
            }
        } else
            return false; /* missing expected digit */
    }
    if (ptr < end && (val[ptr] == 'e' || val[ptr] == 'E')) {
        ++ptr;
        if (ptr < end && val[ptr] == '+')
            ++ptr;
        else if (ptr < end && val[ptr] == '-') {
            exponent_sign = true;
            ++ptr;
        }
        if (ptr < end && val[ptr] >= '0' && val[ptr] <= '9') {
            while (ptr < end && val[ptr] >= '0' && val[ptr] <= '9') {
                if (exponent > (UPPER_BOUND / 10LL))
                    return false; /* overflow */
                exponent = exponent * 10 + val[ptr] - '0';
                ++ptr;
            }
        } else
            return false; /* missing expected digit */
    }
    if (ptr != end)
        return false; /* trailing garbage */

    /* finalize exponent */
    if (exponent_sign)
        exponent = -exponent;
    exponent = exponent - point_ofs + mantissa_tzeros;

    /* finalize mantissa */
    if (mantissa_sign)
        mantissa = -mantissa;

    /* convert to one 64-bit fixed-point value */
    exponent += decimals;
    if (exponent < 0)
        return false; /* cannot represent values smaller than 10^-decimals */
    if (exponent >= 18)
        return false; /* cannot represent values larger than or equal to 10^(18-decimals) */

    for (int i = 0; i < exponent; ++i) {
        if (mantissa > (UPPER_BOUND / 10LL) || mantissa < -(UPPER_BOUND / 10LL))
            return false; /* overflow */
        mantissa *= 10;
    }
    if (mantissa > UPPER_BOUND || mantissa < -UPPER_BOUND)
        return false; /* overflow */

    if (amount_out)
        *amount_out = mantissa;

    return true;
}

static void push(const char* name)
{
    frame* new = malloc(sizeof(frame));

    if (new == NULL) {
        err_printf("push: malloc failed\n");
        exit(EXIT_FAILURE);
    }

    new->name = name;
    new->out_fp = NULL;
    new->state_fd = -1;
    if (curr_frame == NULL)
        new->depth = 1;
    else
        new->depth = curr_frame->depth + 1;

    new->parent = curr_frame;
    curr_frame = new;
}

static void pop()
{
    if (curr_frame == NULL) {
        err_printf("pop: call stack is empty\n");
        exit(EXIT_FAILURE);
    }

    frame* bye = curr_frame;
    if (bye->out_fp != NULL) fclose(bye->out_fp);
    if (bye->state_fd != -1) close(bye->state_fd);
    curr_frame = curr_frame->parent;
    free(bye);
}

/* argv of ourcontract-rt */
static int runtime_argc;
static char** runtime_argv = NULL;

static inline const char* get_contracts_dir()
{
    return runtime_argv[1];
}

int start_runtime(int argc, char** argv)
{
    if (runtime_argv != NULL) {
        err_printf("start_runtime: cannot be called more than once\n");
        return EXIT_FAILURE;
    }

    runtime_argc = argc;
    runtime_argv = argv;

    if (argc < 3) {
        err_printf("usage: ourcontract-rt [CONTRACTS DIR] [CONTRACT] [ARG 1] [ARG 2] ...\n");
        return EXIT_FAILURE;
    }

    in = fdopen(fileno(stdin), "rb");
    out = fdopen(fileno(stdout), "wb");

    return call_contract(argv[2], argc - 2, argv + 2);
}

int call_contract(const char* contract, int argc, char** argv)
{
    char filename[PATH_MAX];
    if (snprintf(filename, PATH_MAX, "%s/%s/code.so", get_contracts_dir(), contract) >= PATH_MAX) {
        err_printf("call_contract: path too long\n");
        return EXIT_FAILURE;
    }
    void* handle = dlopen(filename, RTLD_LAZY);
    if (handle == NULL) {
        err_printf("call_contract: dlopen failed %s\n", filename);
        return EXIT_FAILURE;
    }
    int (*contract_main)(int, char**) = dlsym(handle, "contract_main");
    if (contract_main == NULL) {
        err_printf("call_contract: %s\n", dlerror());
        return EXIT_FAILURE;
    }
    push(contract);
    int ret = contract_main(argc, argv);
    pop();

    dlclose(handle);
    return ret;
}

int err_printf(const char* format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vfprintf(stderr, format, args);

    va_end(args);
    return ret;
}

int str_printf(char* str, unsigned size, const char* format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vsnprintf(str, size, format, args);

    va_end(args);
    return ret;
}

int str_cmp(const char* s1, const char* s2, int n)
{
    return strncmp(s1, s2, n);
}

static int out_open(const char* mode)
{
    char filename[PATH_MAX];
    if (snprintf(filename, PATH_MAX, "%s/%s/out",
            get_contracts_dir(), curr_frame->name) >= PATH_MAX) {
        err_printf("out_open: path too long\n");
        return -1;
    }

    curr_frame->out_fp = fopen(filename, mode);
    if (curr_frame->out_fp == NULL) {
        err_printf("out_open: fopen failed\n");
        return -1;
    }

    return 0;
}

static int out_close()
{
    if (fclose(curr_frame->out_fp) == EOF) {
        err_printf("out_close: fclose failed\n");
        return -1;
    }
    return 0;
}

int out_printf(const char* format, ...)
{
    if (curr_frame->out_fp == NULL) {
        if (out_open("a") == -1) return -1;
    }

    va_list args;
    int ret;

    va_start(args, format);
    ret = vfprintf(curr_frame->out_fp, format, args);

    va_end(args);
    return ret;
}

int out_clear()
{
    if (curr_frame->out_fp != NULL) {
        if (out_close() == -1) return -1;
    }

    if (out_open("w") == -1) return -1;
    return 0;
}

static int state_open(int flags)
{
    char filename[PATH_MAX];
    if (snprintf(filename, PATH_MAX, "%s/%s/state",
            get_contracts_dir(), curr_frame->name) >= PATH_MAX) {
        err_printf("state_open: path too long\n");
        return -1;
    }

    curr_frame->state_fd = open(filename, flags, 0664);
    if (curr_frame->state_fd == -1) {
        err_printf("state_open: open failed\n");
        return -1;
    }

    return 0;
}

static int state_close()
{
    if (close(curr_frame->state_fd) == -1) {
        err_printf("state_close: close failed\n");
        return -1;
    }
    return 0;
}

int state_read(void* buf, int count)
{
    int flag = -1 * count;
    fwrite((void*)&flag, sizeof(int), 1, out);
    fflush(out);
    int ret = fread(buf, count, 1, in);
    return ret;
}

int state_write(const void* buf, int count)
{
    fwrite((void*)&count, sizeof(int), 1, out);
    return fwrite(buf, count, 1, out);
}

int send_money(const char* addr, CAmount amount)
{
    if (strlen(addr) > 40) return -1;
    if (amount < 0) return -1;
    int flag = BYTE_SEND_TO_ADDRESS;
    fwrite((void*)&flag, sizeof(int), 1, out);
    fwrite((void*)addr, sizeof(char), 40, out);
    fwrite((void*)&amount, sizeof(CAmount), 1, out);
    fflush(out);
    return 0;
}

int send_money_to_contract(const char* addr, CAmount amount)
{
    if (strlen(addr) > 64) return -1;
    if (amount < 0) return -1;
    int flag = BYTE_SEND_TO_CONTRACT;
    fwrite((void*)&flag, sizeof(int), 1, out);
    fwrite((void*)addr, sizeof(char), 64, out);
    fwrite((void*)&amount, sizeof(CAmount), 1, out);
    fflush(out);
    return 0;
}

CAmount amount_from_string(const char* val)
{
    CAmount amount;
    if (!parsefixedpoint(val, 8, &amount))
        err_printf("amount_from_string: Invalid amount\n");
    if (!MoneyRange(amount))
        err_printf("amount_from_string: Amount out of range\n");
    return amount;
}