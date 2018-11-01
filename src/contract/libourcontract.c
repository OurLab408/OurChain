#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <linux/limits.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "ourcontract.h"

#define PATH_MAX 4096

/* entry of call stack */
typedef struct _frame {
    const char *name;
    FILE *out_fp;
    int state_fd;
    int depth;
    struct _frame *parent;
} frame;

/* call stack */
static frame *curr_frame = NULL;
FILE *in;
FILE *out;

static void push(const char *name)
{
    frame *new = malloc(sizeof(frame));

    if (new == NULL) {
        err_printf("push: malloc failed\n");
        exit(EXIT_FAILURE);
    }

    new->name = name;
    new->out_fp = NULL;
    new->state_fd = -1;
    if (curr_frame == NULL) new->depth = 1;
    else new->depth = curr_frame->depth + 1;

    new->parent = curr_frame;
    curr_frame = new;
}

static void pop()
{
    if (curr_frame == NULL) {
        err_printf("pop: call stack is empty\n");
        exit(EXIT_FAILURE);
    }

    frame *bye = curr_frame;
    if (bye->out_fp != NULL) fclose(bye->out_fp);
    if (bye->state_fd != -1) close(bye->state_fd);
    curr_frame = curr_frame->parent;
    free(bye);
}

/* argv of ourcontract-rt */
static int runtime_argc;
static char **runtime_argv = NULL;

static inline const char *get_contracts_dir()
{
    return runtime_argv[1];
}

int start_runtime(int argc, char **argv)
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

    return run_contract(argv[2], argc - 2, argv + 2);
}

int run_contract(const char *contract, int argc, char **argv)
{
    char filename[PATH_MAX];
    if (snprintf(filename, PATH_MAX, "%s/%s/code.so", get_contracts_dir(), contract) >= PATH_MAX) {
        err_printf("call_contract: path too long\n");
        return EXIT_FAILURE;
    }

    void *handle = dlopen(filename, RTLD_LAZY);
    if (handle == NULL) {
        err_printf("call_contract: dlopen failed\n");
        return EXIT_FAILURE;
    }

    int (*contract_main)(int, char **) = dlsym(handle, "contract_main");
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

int call_contract(const char *contract, int argc, char **argv)
{
    if (strlen(contract) != 64) {
        return -1;
    }
    int flag = BYTE_CALL_CONTRACT;
    fwrite((void *) &flag, sizeof(int), 1, out);
    fwrite((void *) contract, sizeof(char), 64, out);
    fwrite((void *) &argc, sizeof(int), 1, out);

    int i;
    for (i = 0; i < argc; i++) {
        int argLen = strlen(argv[i]);
        fwrite((void *) &argLen, sizeof(int), 1, out);
        fwrite((void *) argv[i], sizeof(char), argLen, out);
    }
}

int err_printf(const char *format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vfprintf(stderr, format, args);

    va_end(args);
    return ret;
}

int str_printf(char *str, unsigned size, const char *format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vsnprintf(str, size, format, args);

    va_end(args);
    return ret;
}

int str_cmp(const char *s1, const char *s2, int n)
{
    return strncmp(s1, s2, n);
}

static int out_open(const char *mode)
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
    if (fclose(in) == EOF) {
        err_printf("out_close: fclose failed\n");
        return -1;
    }
    if (fclose(out) == EOF) {
        err_printf("out_close: fclose failed\n");
        return -1;
    }
    return 0;
}

int out_printf(const char *format, ...)
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

int state_read(void *buf, int count)
{
    int flag = BYTE_READ_STATE;
    fwrite((void *) &flag, sizeof(int), 1, out);
    return fread(buf, 1, count, in);
}

int state_write(const void *buf, int count)
{
    fwrite((void *) &count, sizeof(int), 1, out);
    return fwrite(buf, 1, count, out);
}

int send_money(const char *addr, long long amount)
{
    if (strlen(addr) > 40) return -1;
    if (amount < 0) return -1;
    int flag = BYTE_SEND_TO_ADDRESS;
    fwrite((void *) &flag, sizeof(int), 1, out);
    fwrite((void *) addr, sizeof(char), 40, out);
    fwrite((void *) &amount, sizeof(long long), 1, out);
    return 0;
}

int send_money_to_contract(const char *addr, long long amount)
{
    if (strlen(addr) > 64) return -1;
    if (amount < 0) return -1;
    int flag = BYTE_SEND_TO_CONTRACT;
    fwrite((void *) &flag, sizeof(int), 1, out);
    fwrite((void *) addr, sizeof(char), 64, out);
    fwrite((void *) &amount, sizeof(long long), 1, out);
    return 0;
}
