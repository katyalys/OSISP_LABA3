#include <sys/types.h>
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>   
#include <limits.h>
#include <errno.h>

void getTime(){
    struct timeval now;
    struct tm* local;

    // получает время
    if (gettimeofday(&now, NULL) == 0){

        local = localtime(&now.tv_sec);         
        printf("PID %d PPID %d Time: %02d:%02d:%02d:%03ld\n\n", getpid(), getppid(), local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000);
    }
    else if (gettimeofday(&now, NULL) == -1){
        perror("Could not get time\n");
    }
}

int main(){

    pid_t child;

    printf("Parent fork:\n");
    getTime();

    printf("Child fork:\n");
    for (int i = 0; i < 2; i++) {
        child = fork();

        // Choosing the child and parent fork
        switch (child) {

            // Output time with child fork
            case 0:
                getTime();
                return 0;
            case -1:
                perror("Fork error");
                exit(-1);
            default:
                break;
        }
    }
    system("ps -x"); 

    while (1){
        if (wait(NULL) == -1){
            if (errno = ECHILD)   
                break;
        }
    }

    return 0;
}
