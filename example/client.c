#include <ourcontract.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

int contract_main(int argc, char **argv)
{
	printf("hello world");
	err_printf("%s: from client!\n", argv[0]);

    return 0;
}
