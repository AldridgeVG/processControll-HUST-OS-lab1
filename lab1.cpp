#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int child1, child2;  // 2 child
int pipeDock[2];     // io port of pipe

void signalHandler1(int signalIn) {
    printf("Parent process recieved cltr+c signal\n");
    if (signalIn == SIGINT) {
        kill(child1, SIGUSR1);  // deliever SIGUSR to chid process
        kill(child2, SIGUSR1);
    }
}

void signalHandler2(int signalIn) {
    //close(pipeDock[0]);
    //close(pipeDock[1]);
    if (child1 == 0 && signalIn == SIGUSR1) {
        printf("Child Process1 is Killed by Parent\n");
        exit(0);
    }
    if (child2 == 0 && signalIn == SIGUSR1) {
        printf("Child Process2 is Killed by Parent\n");
        exit(0);
    }
}

int main() {
    int counter = 1;  // counter
    char buffer[40];
    char info[40];

    // 1. create anonymous pipe and get parent process pid
    printf("Parent pid: %d\n", getpid());
    if (pipe(pipeDock) < 0) {
        printf("pipe initiation failed\n");
        return 0;
    }

    // 2. set soft interrupt signal
    signal(SIGINT, signalHandler1);

    // 3. create child processes
    child1 = fork();
    if (child1 == 0) {
        printf("child1 pid: %d\n", getpid());
        printf("In child1 process\n");
        signal(SIGINT, SIG_IGN);
        signal(SIGUSR1, signalHandler2);
        while (1) {
            close(pipeDock[0]);  // disable read port
            sprintf(info, "I send you %d times", counter++);
            write(pipeDock[1], info, 40);
            sleep(1);  // 1s interval
        }
    } else if (child1 > 0) {  // return to main process to prevent child1 fork
                              // also execute child2=fork()
        child2 = fork();
        if (child2 == 0) {
            printf("child2 pid: %d\n", getpid());
            printf("In child2 process\n");
            signal(SIGINT, SIG_IGN);
            signal(SIGUSR1, signalHandler2);
            while (1) {
                close(pipeDock[1]);  // disable write port
                read(pipeDock[0], buffer, 40);
                printf("%s\n", buffer);
            }
        }

        // wait for process child1,2 to end
        waitpid(child1, NULL, 0);
        //printf("Child Process1 over\n");
        waitpid(child2, NULL, 0);
        //printf("Child Process2 over\n");

        // when childs end, close pipe
        close(pipeDock[0]);
        close(pipeDock[1]);
        printf("Parent Process is Killed\n");
    }
    return 0;
}