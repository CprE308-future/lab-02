---
title: 'Lab 02: Processes and System Calls'
subtitle: 'Iowa State University, CprE 308'
author: 
	- Jeramie Vens
	- Kris Hall
---

# Processes and Interfacing with the Operating System
In this lab you will learn how to create new processes and have those processed do something useful.
Remember that the `man` pages (specifically sections 2 and 3) will be your best friend for this lab.
Some familiarity with how to open a file, read from it, and parse what was read from the file will be needed for this lab as well.

## System call overview
Before proceding we will go over a brief review of what a system call is.
The userspace is what most users commonly interact with on a computer.
This is everything from the terminal shell to the Gnome desktop enviorment to Google Chrome.
The userspace is a multi-user enviorment with restricted permissions and access.
Under the userspace is kernel space: the actual operating system.
Kernel space has complete and total access to everything on the system out of necesity.
Since for obvious reasons it would be bad to give any user access to the kernel space there is a restricted and well defined interface between the two: system calls.
When a system call is made the program execution stops and execution is switched to the operating system address space.
The parameters are passed to the OS through a pre-determined register passing scheme.
The operating system will then check if the program has the permissions to perform the requested operations, and if so completes the task.
The operating system can then return operation to the limited userspace program.
The second section of the linux manual (`man 2`) is dedicated to linux system calls.
For a complete list of all system calls in the current Linux Kenel run `man syscalls`.

Normally you will not need to interact directly with system calls, but instead will use library calls provided in Standard C Library (`libc`).
In most cases this library calls are identical or nearly identical to the underlying system calls, but handle some of the intricacies of the actual call.
The third section of the linux manual (`man 3`) is dedicated to the Standard C Library.
Many general topics are covered in the seventh section of the linux manual (`man 7`).
As an example, look at the difference between `man 2 pipe`, `man 3 pipe`, and `man 7 pipe`.
You will learn about pipes in a future lab, so don't wory about the details right now.

# Creating a new process
Recal that to create a new child process in a C program the following lines of code are needed:

~~~C
#include <unistd.h>
// ...
pid_t child;
child = fork();
// ...
~~~

This snippet will create a variable name `child` and store the return value of the `fork()` system call which is where all the magic happens.
This system call creates a new process as a child of the current process that is a clone of the current process.
This means that the child process has the same program, program counter (where in the program the process is), data, etc. (with some exception).

Refer to

~~~bash
$ man 2 fork
~~~

for the full list of exceptions that the parent process has at the time the `fork()` call is made.

Use the fork man page to answer the following questions in your lab report

  - What are the return values of `fork()`?
  - How can a program determine if it is the parent or child process?

## Make the new process do something different
Since the `fork()` system call creates a clone of the parent process the child process must use the return value from `fork()` to do something different than the parent.
Remeber that `fork()` return is zero for the child and positive for the parent.  A common code snippit for determining if this is the child or parent is shown below.

~~~c
pid_t child;
child = fork();
if ( 0 == child ) {	        // if child == 0
	// Child code here
} else if ( 0 < child) {    // if child > 0
	// parent code here
} else {                    // if child < 0
	// fork failed, handle error here
	perror("fork");
}
~~~

## Waiting on the child process
In the above example the parent and child code execute independently of each other and either can finish first.
In the event that this is not the desired behavior (e.g. almost always), there are two system calls that the parent can run that will guarantee the parent waits for the child process to exit.  The two functions are `wait` and `waitpid`.  Full information on each of these system calls can be found at

~~~bash
$ man 2 wait
~~~

Below is a code snippit that uses `wait` to make the parent process wait for the child process to complete.

~~~c
int status = 0;
pid_t child = fork();
if ( 0 == child){
	printf("I am the child!\n");
	sleep(5);
	return 42;
} else if ( 0 < child ) {
	wait(&status);
	printf("Child process is done, status is: %d\n", WEXITSTATUS(status));
	return 0;
} else {
	perror("fork");
	exit(-1);
}
~~~

This snippet will make sure that the parent suspends execution until one of its children terminates.  In the event that there are multiple children, and knowedge of a specific child process' termination is of importance, then `waitpid` should be used instead. 
The code snippit to do that is:

~~~c
int status = 0;
pid_t child = fork();
if ( 0 == child){
	printf("I am the child!\n");
	sleep(5);
	return 42;
} else if ( 0 < child ) {
	waitpid(child, &status, 0);
	printf("Child process is done, status is: %d\n", WEXITSTATUS(status));
	return 0;
} else {
	perror("fork");
	exit(-1);
}
~~~

This will guarantee that the parent process waits for the child process with the process id stored in `child` to terminate before continuing.
Note that, as shown in the man page for `wait`, to make this snippet of code work like the previous cnippet, change the value of the first argument of `waitpid` to `-1`.

It is highly recommened that the student read through the man page for `wait` and `waitpid` to fully understand how to use the `wait` and `waitpid` system calls, and experiment with multiple `fork` and `wait` programs until fully comfortable with what is actually happening.
What is presented here is merely a quick overview of how to use the two.

In your lab report answer the following questions:

  - How can you use `waitpid` to determine if a child process ended without making the parent wait for that child to end?
  - What is the return value of `waitpid`?
  

# Executing new programs
Fork is useful for splitting a process into two, but it is not very useful on its own to execute new programs.
To start a new program a different system call must be used: `execve`.
The basic prototype of the `execve` system call is:

~~~c
#include <unistd.h>
int execve(const char * filename, char * const argv[], char * const envp[]);
~~~

When executed the system call will execute the program at `filename` with the arguments `argv` and environment variables `envp`.
This will replace the calling processes text, bss, and stack with the new proces.
On a successful call the `execve` will not return to the calling process.

To learn more about the `execve` system call and the `exec` library calls look at the following man pages:

~~~bash
$ man 2 execve
$ man 3 exec
~~~

Usually it is easier to use the `exec` library calls over the system call (this is generally the case with all system calls).
For example, the `execvp` will search the `PATH` variable to find the executable, and copy the environment variables from the calling process to the new process.
As an example, the following code will execute gedit to create a new file.

~~~c
char * args[] = {"gedit", "hello.c", NULL};
execvp(args[0], args);
perror("execvp");
~~~

# Tying it together
With this information we can now make some more useful programs.
Suppose that a complex function is needed to be completed by the child, but this function has already been implemented by somebody else.
Following the UNIX philosopy, we should call the existing code instead of reimplement it ourselves.
For example, the terminal emulator you are using does this.
Everytime you run a command it calls `fork` and then in the child process calls `exec` to run that program and the parent calls `wait` to wait for the child to finish.
Below is pseudocode for a terminal shell:

~~~
loop forever
	get command from user
	call fork
	if child
		execute command
	if parent
		wait for child to finish
end
~~~

# Example Program
Now that you have seen some general examples it is time to see a more concrete example.
This will consist of two programs: one that prints out a list of passed arguments and one that executes the first program.

## Command argument printer
Examine the `arg-printer.c` file in the labs folder.
This program prints out all arguments passed to it on the command line and returns the number of arguments it recieved.
Compile the file and try running it with the following commands (note the outputs in your lab report):

~~~bash
$ gcc -o arg-printer arg-printer.c
$ ./arg-printer
$ ./arg-printer a b c
$ ./arg-printer --version
~~~

## Fork and exec example
Next look at the `example.c` file in the lab folder.
This file calls the `arg-printer` program and passes in three arguments to it.
Compile it and test with the following commands.
Include the output in your lab report.

~~~bash
$ gcc -o example example.c
$ ./example
~~~

# Tasks for this lab
With the knowledge of how to create a child process, and make the child run a different program, the task for this student for this lab is to create a program that when given the path to a file, opens the file with the appropriate software.
For example, a `.c` file should be opened with `gedit`, and `.pdf` should be opened with `evince`.
The specification of the program is as follows:

  1. The program should take one command line argument: the file to be opened
  2. If there is no argument, of the file that is passed does not have a program that can open it for viewing, a usage message should be printed and the program should exit.
  3. In the event that an error occurred, there should be an error message associated with it (use `perror`), and the program should exit; there should be no segmentation faults.
  4. The program is required at a minimum to support the following file types and open with these programs:
    1. x.doc -> libreoffice
    2. x.odt -> libreoffice
    3. x.png -> eog
    4. x.txt -> gedit
    5. x.c -> gedit
    6. x.pdf -> evince
    7. x.mp3 -> vlc
  5. The parent process should wait on the child process (the process that opens the file with the correct program) to terminate and print out the termination status.
  6. **Extra credit** will be given for adding additional file types through a runtime configuration file (`~/.config/anyopen/file-types.conf`).
  7. **Extra credit** will be given for opening multiple files at the same time and printing the exit status of each program separately.
  
Example usage:

~~~bash
$ ./anyopen lab-02.pdf
~~~

# License
This lab write up and all accompaning material are distributed under the MIT License.
For more information, read the accompaning LICENSE file distributed with the source code.
    
















