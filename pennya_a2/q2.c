#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void handle(int arg) 
    {
	printf("%d signal started, (%d)\n", getpid(), arg);
    sleep(5);
    printf("%d signal complete, (%d)\n", getpid(), arg);
    exit(1);
    }

int main(void) 
    {
    pid_t pid = fork();
    signal(SIGUSR1, handle);
    if (!pid)
        {
        int pidParent = getpid() - 1;
        printf("\rIn child (%d), sending same signal that parent is executing to parent (%d)\n", getpid(), pidParent);
        kill(pidParent, SIGUSR1);
        }
    else
        {
        printf("In parent (%d), sending signal\n", getpid());
        sleep(0);
        kill(getpid(), SIGUSR1);
        exit(1);
        }
	return 0;
    }

