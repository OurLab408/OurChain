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

static int foo(int i) {
    char proof[100];
    sprintf(proof, "ourZK/proof%d.json", i);
    return private_zokrates_verify(
        "./ZoKrates/target/release",
        proof,
        "ourZK/verification.key"
    );
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

    /* subcommand "add" */
    if (str_cmp(argv[1], "add", 3) == 0) {
        if (argc != 3) {
            err_printf("%s: usage: add {number}\n", argv[0]);
            return 0;
        }

        out_clear();
        int result = foo(atoi(argv[2]));
        state.user_count += atoi(argv[2]);
        out_printf("Count: %d. Result: %d.", state.user_count, result);

        state_write(&state, sizeof(state));
        return 0;
    }

    return 0;
}
