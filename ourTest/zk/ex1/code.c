#include <ourcontract.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static struct {
    char name[100];
} state;

static void state_init() {
    state.name[0] = '\0';
    out_clear();
}

static int foo (char *a, char *b1, char *b2, char *c) {
    char **inputs;
    return zokrates_verify(a, b1, b2, c, inputs, 0);
}

/*
 * argv[0]: contract id
 * argv[1]: subcommand
 * argv[2...]: args
 */
int contract_main(int argc, char **argv) {
    if (state_read(&state, sizeof(state)) == -1) {
        /* first time call */
        state_init();
        state_write(&state, sizeof(state));
        state_read(&state, sizeof(state));
    }

    if (argc < 2) {
        err_printf("%s: no subcommand\n", argv[0]);
        return 0;
    }

    /* subcommand "foo" */
    if (str_cmp(argv[1], "foo", 3) == 0) {
        if (argc != 2 + 5) {
            err_printf("%s: usage: foo {name} {a} {b1} {b2} {c}\n", argv[0]);
            return 0;
        }

        out_clear();
        if(!foo(argv[3], argv[4], argv[5], argv[6]))
            out_printf("Proved by %s.", argv[2]);

        state_write(&state, sizeof(state));
        return 0;
    }

    return 0;
}
