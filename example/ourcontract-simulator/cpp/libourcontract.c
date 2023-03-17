#define PATH_MAX 128
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "ourcontract.h"

/* argv of ourcontract-rt */
static int runtime_argc;
static char **runtime_argv = NULL;

static inline const char *get_contracts_dir()
{
    return runtime_argv[2];
}

int start_runtime(int argc, char **argv)
{
    if (runtime_argv != NULL)
    {
        err_printf("start_runtime: cannot be called more than once\n");
        return EXIT_FAILURE;
    }

    runtime_argc = argc;
    runtime_argv = argv;

    if (argc < 4)
    {
        err_printf("usage: ourcontract-rt [CONTRACTS DIR] [CONTRACT] [ARG 1] [ARG 2] ...\n");
        return EXIT_FAILURE;
    }

    return call_contract(argv[3], argc - 3, argv + 3);
}

int call_contract(const char *contract, int argc, char **argv)
{
    char filename[PATH_MAX];
    if (snprintf(filename, PATH_MAX, "%s/code.so", get_contracts_dir()) >= PATH_MAX)
    {
        err_printf("call_contract: path too long\n");
        return EXIT_FAILURE;
    }
    void *handle = dlopen(filename, RTLD_LAZY);
    if (handle == NULL)
    {
        err_printf("call_contract: dlopen failed\n");
        return EXIT_FAILURE;
    }
    int (*contract_main)(int, char **) = dlsym(handle, "contract_main");
    if (contract_main == NULL)
    {
        err_printf("call_contract: %s\n", dlerror());
        return EXIT_FAILURE;
    }
    int ret = contract_main(argc, argv);
    dlclose(handle);
    return ret;
}

int err_printf(char *str, ...)
{
    FILE *fptr = fopen("./storage/err.log", "a");
    if (fptr == NULL)
    {
        printf("can not access ./storage/err.log");
        return 1;
    }
    printf("Error:%s", str);
    fprintf(fptr, "Error:%s", str);
    fclose(fptr);
    return 0;
}

int str_printf(char *str, unsigned size, const char *format, ...)
{
    printf("console:%s", str);
    return 0;
}

int str_cmp(const char *s1, const char *s2, int n)
{
    return strncmp(s1, s2, n);
}

int state_read(void *buf, int count)
{
    char filename[PATH_MAX];
    if (snprintf(filename, PATH_MAX, "./storage/state.txt") >= PATH_MAX)
    {
        err_printf("state_open: path too long\n");
        return -1;
    }
    int state_fd = open(filename, O_WRONLY | O_CREAT, 0664);
    if (state_fd == -1)
    {
        err_printf("state_open: open failed\n");
        return -1;
    }
    int result = read(state_fd, buf, count);
    if (close(state_fd) == -1)
    {
        err_printf("state_close: close failed\n");
        return -1;
    }
    return result;
}

int state_write(const void *buf, int count)
{
    char filename[PATH_MAX];
    if (snprintf(filename, PATH_MAX, "./storage/state.txt") >= PATH_MAX)
    {
        err_printf("state_open: path too long\n");
        return -1;
    }
    int state_fd = open(filename, O_WRONLY | O_CREAT, 0664);
    if (state_fd == -1)
    {
        err_printf("state_open: open failed\n");
        return -1;
    }
    int result = write(state_fd, buf, count);
    if (close(state_fd) == -1)
    {
        err_printf("state_close: close failed\n");
        return -1;
    }
    return result;
}
