#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, const char * argv[])
{
	char * args[] = {"gedit", "hello.c", NULL};
	printf("Opening %s with file %s\n", args[0], args[1]);
	execvp(args[0], args);
	perror("execvp");
	return -1;
}
