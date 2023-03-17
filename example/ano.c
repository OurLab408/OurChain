#include <stdio.h>
#include <unistd.h>

int main()
{
	char *par[] = {"ls", NULL};
	execv("/bin/ls", par);
}