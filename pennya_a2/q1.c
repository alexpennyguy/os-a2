//--------------------------------------------
// Class: pipes.c
//
// Author: Alex Penny
//
// Description: Send output between processes with pipes,
//              executes a program and messes with
//              it's output.
//
// Professor: Franklin Bristow
//--------------------------------------------

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <stdbool.h> 
#include <sys/wait.h>
#include <sys/types.h>
#include <assert.h>

#define WRITE_END 1
#define READ_END 0

#define WORD_SIZE 3
#define BUFFER_SIZE 2
#define NO_OF_CHARS 1

#define PIPE_COUNT 2

#define CHILD 0

char* wordToReplace;
char* wordToReplaceCapital;

char* replacementWord;

bool replacing;

//--------------------------------------------
// handler
//
// Description: Accepts a signal and changes
//              values depending on
//              what that signal is.
//--------------------------------------------

void handler(int signum)
    {
    assert(signum);

    if (signum == SIGUSR1)
        {
        replacing = true;
        wordToReplace = "dog";
        wordToReplaceCapital = "DOG";
        replacementWord = "cat";
        }
    
    else if (signum == SIGUSR2)
        {
        replacing = true;
        wordToReplace = "cat";
        wordToReplaceCapital = "CAT";
        replacementWord = "dog";
        }
    
    else
        {
        replacing = false;
        wordToReplace = NULL;
        wordToReplaceCapital = NULL;
        replacementWord = NULL;
        }

    }

//--------------------------------------------
// main
//
// Description: Accepts command line input,
//              executes a program, and communicates
//              between processes with pipes.
//--------------------------------------------

int main (int argc, char **argv)
    {
    int pipefd[PIPE_COUNT];
    pid_t pid;
    char buf[BUFFER_SIZE];
    char buf2[WORD_SIZE];
    ssize_t amt_read;

    //Default values

    wordToReplace = malloc(sizeof(char*)*WORD_SIZE);
    wordToReplace = "cat";
    wordToReplaceCapital = malloc(sizeof(char*)*WORD_SIZE);
    wordToReplaceCapital = "CAT";

    replacementWord = malloc(sizeof(char*)*WORD_SIZE);
    replacementWord = "dog";

    replacing = true;

    if (argc <= 1)
        {
        fprintf(stderr, "Needs the name of a program to execute.\n");
        exit(EXIT_FAILURE);
        }

    if (pipe(pipefd) == -1) 
        {
        perror("Pipe creation failure");
        exit(EXIT_FAILURE);
        }

    pid = fork();

    if (pid < CHILD) 
        {
        // fork failed; exit
        fprintf(stderr, "fork failed\n"); 
        exit(EXIT_FAILURE);
        }

    if (pid == CHILD)
        {    
        //Child reads from pipe
        close(pipefd[READ_END]);
        
        assert(argc > 1); // Pre-condition

        char *myargs[argc];
        
        for (int i = 0; i < argc ; i++ )
            {
            myargs[i] = argv[i+1];
            
            if (i == argc - 1)
                {
                myargs[i] = NULL;
                }

            }

        if (dup2(pipefd[WRITE_END], STDOUT_FILENO) == -1)
            {
            fprintf(stderr, "dup2 failure\n");
            exit(EXIT_FAILURE);
            }

        assert(myargs != NULL); //Post-condition

        if(execvp(myargs[0], myargs) == -1)
            {
            fprintf(stderr, "Execution failure\n");
            exit(EXIT_FAILURE);
            }

        close(pipefd[WRITE_END]);
        exit(EXIT_SUCCESS);
        }
         
    else
        {           
        signal(SIGUSR1, handler);
        signal(SIGUSR2, handler);
        signal(SIGALRM, handler);
        
        while(1)
            {
            close(pipefd[WRITE_END]);
            
            amt_read = read(pipefd[READ_END], &buf, NO_OF_CHARS);
            
            while (amt_read > 0)
                {
                assert(amt_read > 0); //ensure read worked correctly

                if (replacing && (buf[0] == wordToReplace[0] || buf[0] == wordToReplaceCapital[0]))
                    {
                    buf2[0] = wordToReplace[0];
                    amt_read = read(pipefd[READ_END], &buf, NO_OF_CHARS);
                    
                    if (buf[0] == wordToReplace[1] || buf[0] == wordToReplaceCapital[1])
                        {
                        buf2[1] = wordToReplace[1];
                        amt_read = read(pipefd[READ_END], &buf, NO_OF_CHARS);
                        
                        if (buf[0] == wordToReplace[2] || buf[0] == wordToReplaceCapital[2])
                            {
                            buf2[0] = replacementWord[0];
                            buf2[1] = replacementWord[1];
                            buf2[2] = replacementWord[2];
                            write(STDOUT_FILENO, &buf2, WORD_SIZE);
                            amt_read = read(pipefd[READ_END], &buf, NO_OF_CHARS);
                            }
                        
                        else
                            {
                            write(STDOUT_FILENO, &buf2, NO_OF_CHARS + 1);
                            amt_read = read(pipefd[READ_END], &buf, NO_OF_CHARS);
                            }

                        }
                    
                    else
                        {
                        write(STDOUT_FILENO, &buf2, NO_OF_CHARS);
                        write(STDOUT_FILENO, &buf, NO_OF_CHARS);
                        amt_read = read(pipefd[READ_END], &buf, NO_OF_CHARS);
                        }

                    }
                
                else
                    {
                    buf[1] = '\0';
                    write(STDOUT_FILENO, &buf, NO_OF_CHARS);
                    amt_read = read(pipefd[READ_END], &buf, NO_OF_CHARS);
                    }

                }

            write(STDOUT_FILENO, "\n", NO_OF_CHARS);
            close(pipefd[READ_END]);
            exit(EXIT_SUCCESS);
            }

        }

    return 0;
    }
