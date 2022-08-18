// compile with gcc *.c -o minishell

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
// using a modified linked list implementation from Lab 2 that takes in char* as data
#include "linkedlist.h"

// maximum size of the buffer for one line of command line input
#define MAX_BUFFER_SIZE 80

// Create a signal handler
void terminate(int sig){
	// FIXME should I use write(1, "some text", 35) instead? What is the difference?
	// FIXME is it possible to always print the terminating message on a new line, whether it's called by typing "exit" or by ^C ?
	// FIXME might need to implement popping all the running child processes (in case some of the have been orphaned) - use a stack (in 'runCommand' add new forks to the stack, and, when the processes completed, pop it off)  
	printf("❌ mini-shell terminated\n");
	exit(0);
}

// Default mini shell "help" command implementation that displays information about the mini shell and it's default commands
void helpDefault(char* ignored){
	printf("\n==============================================================================\n");
	printf("👋 This is a mini-shell\n\n");
	printf("Type in any bash standard bash command to run it.\n\n");
	printf("Piping only works for two operands. Adding more operands to the pipe will not crash the program, but the input won't be transfered beyond the second operand\n\n");
	printf("Here are the default commands, implemented for this shell:\n");
	printf("help - you are here 👈\n");
	printf("cd - change directories; works like expected, but without autocompletion\n");
	printf("history - shows the complete history of all the comands run through this shell\n");
	printf("exit - terminate this shell\n");
	printf("==============================================================================\n\n");
}

// Default mini shell "exit" command implementation that terminates the mini shell
void exitDefault(char* ignored){
	terminate(0);
}

// Default mini shell "cd" command implementation that changes the current directory where the mini shell runs
void cdDefault(char* path) {
	// we run chdir() and if it returns anything but the success status, we print an error
	if (chdir(path) != 0) { 
		printf("cd: could not change directory to %s\n", path);
	}
}

// Declare a linked list to store command history
static linkedlist_t* history_ll;

// Default mini shell "history" command implementation that prints out the history of all the commands that a user invoked  with the current instance of the shell
void historyDefault(char* ignored) {
	printf("Here's a history of commands:\n");
	// using the previously implemented printing function for the linked list
	PrintLinkedList(history_ll);
}

// Define a list of standard commands and their names
static char* commandNames[] = {"help", "exit", "cd", "history"};
static void(*commandFunctions[])(char*) = {helpDefault, exitDefault, cdDefault, historyDefault};

// Checks if there is a command with a given list in the given array of command names
// If the command exists, the function runs it and returns 0 for successful completion
// If there is no such command in the array, returns -1
int tryDefault(char* args[]){
	// calculate the length of the array of the commands
	int length = sizeof(commandNames)/sizeof(char*); 
	// define the command name
	char * name =  args[0];
	int i;
	for (i=0; i< length; i++){
		if (strcmp(commandNames[i], name) ==0) {
			commandFunctions[i](args[1]);
			return 0;
		}
	}
	// returning -1, if the default command is not found
	return -1;

}

// Runs a given command (it is expected to be defined in argv[0]) with provided arguments 
// if pipeOut argument is 1, the command is going to be output to a pipe
void runCommand(char* argv[], int pipeOut){
	if (NULL==argv[0]){
		printf("⚠️  Name of the command is not specified for one of the commands. Try again\n");
		return;
	}		
	
	// Declare an array to store file descriptors for piping
	static int fd[2];

	// Declare a status variable;
	// It flags, whether the current function call has a pipe input to input from
	static int pipeIn = 0;

	// Declaring the variable for the child process id
	pid_t p;

	// we open a new pipe if this is a first call of runCommand with a pipeOut flag set to true
	if (1==pipeOut && 0==pipeIn){
		// checking that opening the pipe does not result in an error
		if (pipe(fd) == -1) {
			fprintf(stderr, "Pipe Failed");
			return;
		}
	}


	// if pipeOut is true (1), then we output the result of the given command to the pipe
	if (1 == pipeOut){
		if((p = fork())==0 ){
			// we set the input from pipe if the pipeIn is true (1)
			if (1==pipeIn) {
				// closing standard input
				close(STDIN_FILENO);	
				// set up the input from the pipe
				dup2(fd[0], STDIN_FILENO);
			} 
			// we do not need the input from the pipe anymore so we close it
			close(fd[0]);
			// we close the standard output
			close(STDOUT_FILENO);
			// and output to the pipe
			dup2(fd[1], STDOUT_FILENO);
			// if the default command successfuly executed, we complete execution
			// we do not need the output to the pipe anymore so we close it
			close(fd[1]);
		    // trying to run the default commands
            if (0==tryDefault(argv)) {
				exit(0);
			};		
			// otherwise we try to run the given command as an executable process	
			// Executes command from child then terminates our process
			execvp(argv[0],argv);
			// If exec terminates unsuccessfully, we display an error
			printf("Cannot execute %s. Command not found or an argument error.\n", argv[0]);
			exit(1);
		} else if (p < 0){
			// handling the error when creating a fork
			fprintf(stderr, "Fork failed");
		} else {
			// Handling the parent process
			wait(NULL);
			// we do not need the writing end of the pipe anymore
			close(fd[1]);
			// we set up the next command to receive the input from the pipe
			pipeIn = 1;
		}

	} else 
	// running the command that does not output to the pipe
	{
		// trying to find and run a default mini shell command
		// if the default command successfuly executed, we return
		if (0==tryDefault(argv)) {
			return;
		};
		// otherwise we try to run the given command as an executable process	
		if(0==(p = fork())){
			// Handling the child process
			// we set the input from pipe if the pipeIn is true (1)
			if (1==pipeIn) {
				// closing standard input
				close(STDIN_FILENO);	
				// set up the input from the pipe
				dup2(fd[0], 0);
			    // closing file descriptors
                //close(fd[0]);
				close(fd[1]);
			}
			// Executes command from child then terminates our process
			execvp(argv[0],argv);
			// If exec terminates unsuccessfully, we display an error
			printf("⚠️  Cannot execute %s. Command not found or an argument error.\n", argv[0]);
			exit(1);
		} else if (p < 0) {
			// handling the error when creating a fork
			fprintf(stderr, "Fork failed");
            return;
		} else {
			// Handling the parent process
			wait(NULL);
			// we close the pipe input, if it was previously open
			if (1==pipeIn) {close(fd[0]);}
			// this is the last operation in the pipe, so we set pipeIn back to 0
			pipeIn = 0;
		}
	}
}



int main(){
	alarm(180); 
	// Please leave in this line as the first statement in your program.
	// This will terminate your shell after 180 seconds,
	// and is useful in the case that you accidently create a 'fork bomb'

	// Install our signal handler
	signal(SIGINT, terminate);

	// defile the variable where the Command Line input is store	
	char line[MAX_BUFFER_SIZE];

	// initialize the history linked list
	history_ll  = CreateLinkedList();


	// A loop that runs forever.
	while(1){
		// Find an absolute path to current directory	
		char* pwd = getcwd(NULL, 0);
		// Find the name of the current directory (it comes after the last '/' in the absolute path)
		char * str = strrchr(pwd, '/') + 1;
		// Freeing the pointer to the absolute path string
		free(pwd);
		printf("mini-shell 🐚  %s > ", str);
		
		// Read in 1 line of text
		// The line is coming from 'stdin' standard input
		fgets(line,MAX_BUFFER_SIZE,stdin);	

		// adding the current input to the history linked list
		AppendToLinkedList(history_ll, line);

		// We can store up to 16 arguments.		
		// We initialize the array of args to nulls so that the last element after all the string arguments is always null (that's needed for exec to work properly)
		char* myargv[16] = {NULL};	
		// We split the input string into tokens
		char* arg = strtok(line, " \n");
		// We count the number of arguments in the input string
		int count = 0;
		while (NULL != arg) {
			// we cannot store more than 1 command and 15 arguments
			if (count > 15) {
				perror("mini-shell cannot accept more than 15 arguments for one command\n");
				break;
			}

			// checking for a pipe symbol	
			if (strcmp("|", arg)==0){
				// running the command in a pipe
				// 1 is a pipe flag
				runCommand(myargv, 1);
				// resetting the argument array for the next command after the pipe
				memset(myargv, 0, 16);	
				// resetting the counter for the next command after the pipe
				count = 0;
			} else {
				// store the current string token as a CLI argument
				myargv[count] = arg;
				count++;

			}

			// parsing the next string token
			arg = strtok (NULL, " \n");
		}	      
	
		// running the command with all the given arguments stored in myargv array
		runCommand(myargv, 0);	
		continue;	

	}

	return 0;
}
