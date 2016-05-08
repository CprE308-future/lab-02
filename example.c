#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, const char * argv[])
{
	char * arguments[10];
	pid_t child;
	
	arguments[0] = "./arg-printer";
	arguments[1] = "arguments";
	arguments[2] = "are";
	arguments[3] = "useful";
	arguments[4] = NULL; // make sure to null terminate the arg list.

	child = fork();
	if(0 == child)
	{
		execvp(arguments[0], arguments);
		printf("Something went wrong!\n");
	}
	else if(0 < child)
	{
		int status;
		waitpid(child, &status, 0);
		printf("Child process finished with return code %d\n", WEXITSTATUS(status));
	}
	else
	{
		perror("fork");
	}
	return 0;
}
