#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, const char * argv[])
{
	int status = 0;
	pid_t child = fork();
	
	if ( 0 == child) {
		printf("I am the child!\n");
		sleep(5);
		return 42;
	} else if ( 0 < child ) {
		waitpid(child, &status, 0);
		printf("Child %lu is done, status is %d\n", child, WEXITSTATUS(status));
		return 0;
	} else {
		perror("fork");
		return -1;
	}
}
