#include <ourcontract.h>

int contract_main(int argc, char **argv)
{
    if(argc > 2)
        err_printf("%s: %d\n", argv[0], send_money_to_contract(argv[1],atoi(argv[2])));
    return 0;
}
