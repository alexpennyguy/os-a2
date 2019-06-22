# Assignment 2 for COMP3350
Assignment 2

# Question 1 - Pipes ’n signals
## Description
In this question, you’ll be writing a multi-process program that simulates a man in the middle attack using pipes, signal handlers, and fork/exec to launch a program.

You will write a program that will launch a different program and interfere with the standard output of that program.

## Implementation
At a high level, the program you write should do the following:

1. Accept command-line arguments, where the command-line argument(s) are the program that should be later exec-ed by your fork-ed process.
2. Create an anonymous pipe for communication.
3. fork a child process.
## … in the child process
4. Link standard output to the write end of the pipe with dup2 (see man dup2). Note that you will need to pass a file number, which is defined as STDOUT_FILENO for standard output.
5. Launch the program specified on the command-line using execvp (see man execvp). When you call execvp, note that the first argument to any running program is the name of the program.
## … in the parent process
6. Register at least 3 signal handlers:
- A handler that will modify behaviour to replace any instance of the word dog with cat when SIGUSR1 is received.
- A handler that will modify behaviour to replace any instance of the word cat with dog when SIGUSR2 is received.
- A handler that will go back to normal (no text is replaced) when SIGALRM is received.
7. The default behaviour should be to replace cat with dog.
read the read end of the pipe (note that you should specifically be using read) into a buffer until you know the text isn’t what you need to replace, or the “no replacement” mode is enabled. This is going to work best if you read one byte/character of data at a time.
8. write modified (or unmodified) data to standard output on the parent process.

# Question 2 - Signals implementation
## Description
In this question you’ll be taking a look at how signal handlers behave and are implemented at the kernel level. The main question you’re going to be answering in this question is: What happens if a signal handler gets invoked when it’s already handling a signal with that same handler?
