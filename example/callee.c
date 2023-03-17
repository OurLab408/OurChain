#include <ourcontract.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct Local_value{
	int int_array[10];
	char char_array[10];
} local_value;

typedef struct Data{
	local_value var;
	int a;
} data;
static data state;



int init_step(data * state){
	state->a = 0;
	
	return 0;
}

int operate_step(data * state){
	state->a += 1;

	return 0;
}

int my_call_contract(const char *contract, int argc, char **argv)
{
        char arg_string[800] = "";
        for (int i=1; i<argc; i++){
                strcat(arg_string, argv[i]);
                strcat(arg_string, " ");
        }
        char cmd[1000];
        sprintf(cmd, "bitcoin-cli -regtest callcontract %s %s &", contract, arg_string);

        err_printf("call %s: %s\n", contract, cmd);
        system(cmd);

        return 0;
}



int contract_main(int argc, char **argv)
{
	/*
	 *	argv[1]:goto program block id
	 *	argv[2]:self deploy id  (for callcontract return)
	 *	argv[3]:call callee deploy id (for call callee)
	 */
	switch (atoi(argv[1])){

		case 0:{
			/*	for contract deploy
			 *	do nothing
			 *	get id in terminal
			 */
    			err_printf("%s: deploy\n", argv[0]);
			break;
		}
		case 1:{
			//callee
    			err_printf("%s: case 1\n", argv[0]);

    			err_printf("%s: callee run\n", argv[0]);
			//if (state_read(&state, sizeof(state)) == -1) {
			//	init_step(&state);
		        //	state_write(&state, sizeof(state));
		        //	state_read(&state, sizeof(state));
		    	//}
			//
    			//err_printf("%s: state.a = %d\n", argv[0], state.a);

			//state.var.int_array[0] = 1;
			//sprintf(state.var.char_array, "%s\n", "char");

			int call_argc = 5;
			char* call_argv[call_argc];
			call_argv[1] = argv[4];	//goto which callee block
			call_argv[2] = argv[3]; //tell callee what callee id is
			call_argv[3] = "empty";
			call_argv[4] = "return_msg";     
			my_call_contract(argv[3], call_argc, call_argv);
			break;
		}
		case 2:{
			//operate
    			//err_printf("%s: case 2\n", argv[0]);

			//state_read(&state, sizeof(state));
			//operate_step(&state);
			//state_write(&state, sizeof(state));
			//state_read(&state, sizeof(state));

    			//err_printf("%s: state.a = %d\n", argv[0], state.a);

			break;
		}
		default:{
    			err_printf("%s: default\n", argv[0]);
			break;
		}
	}

    	return 0;
}
