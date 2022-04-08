#include <sys/types.h>
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>   

void getTime(char* num){
    struct timeval now;
    struct tm* local;

    gettimeofday(&now, NULL);
    local = localtime(&now.tv_sec);

    printf("%s PID %d PPID %d Time: %02d:%02d:%02d:%03ld\n\n",  num, getpid(), getppid(), local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000);
}

void waitChild(pid_t pid) {
	if (waitpid(pid, NULL, 0) == -1)
		perror("wait pid failure\n");
}

void main(){
    
    pid_t pid1, pid2;

    pid1 = fork();
    pid2 = fork();
    if (pid1 > 0 && pid2 > 0){
        getTime("Parent: ");

        system("ps -x");

        waitChild(pid1);
        waitChild(pid2);
    }

    if (pid1 == 0 && pid2 > 0)
    {
        getTime("First child: "); 
    }

    if (pid1 > 0 && pid2 == 0)
    {
        getTime("Second child: ");
        
    }

    if (pid1 < 0){
        perror("1st child couldn't be created");
    }
    else if (pid2 < 0){
        perror("2nd child couldn't be created");
    }


}