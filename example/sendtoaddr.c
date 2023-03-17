#include <ourcontract.h>

int contract_main(int argc, char **argv)
{
    if(argc > 2)
        err_printf("%s: %d\n", argv[0], send_money(argv[1],amount_from_string(argv[2])));
    return 0;
}
