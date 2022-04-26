#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <alloca.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <errno.h>

#define BUFSIZE 4096*64

long count = 0; 

int CopyFile(const char *currName1, const char *currName2, int *bytesCount) {
    mode_t mode;
    struct stat fileInfo;

    mode = fileInfo.st_mode;

    errno = 0;
    int fs = open(currName1,  O_RDONLY); //file source
    if (errno != 0){
        perror("Error opening the source file\n");
        return -1;
    }

    errno = 0;
    int fd = open(currName2, O_WRONLY|O_CREAT, mode); //file destination
    if (errno != 0){
        perror("Error opening the destination file\n");
        return -1;
    }

    *bytesCount = 0;

    char *buff = (char*) malloc(BUFSIZE);
    ssize_t readFile, writeFile;
    while (((readFile = read(fs, buff, BUFSIZE)) != 0) && (readFile != -1)) {

        if ((writeFile = write(fd, buff, (size_t)readFile)) != -1){
            *bytesCount = *bytesCount + (int)writeFile;
        }

    }
    free(buff);

    if (close(fs) || close(fd)){
        perror("Error closing file\n");
        return -1;
    }

    return 0;
}

int Process(char *dir1, char *dir2, long maxCount){

    //int count = 0;
    struct stat st1, st2;


    errno = 0;
    DIR *cd1 = opendir(dir1);
    if (cd1 == NULL){
        fprintf(stderr,"Could not open directory: %s \n", strerror(errno));
        return -1;
    }

    struct dirent *entry = alloca(sizeof(struct dirent));
    if (errno != 0){
        perror("Could not open directory\n");
    }

    // first directory
    char *currName1 = alloca(strlen(dir1) + NAME_MAX);
    currName1[0] = 0;
    strcat(currName1, dir1);
    strcat(currName1, "/");

    // second directory
    char *currName2 = alloca(strlen(dir2) + NAME_MAX);
    currName2[0] = 0;
    strcat(currName2, dir2);
    strcat(currName2, "/");

    size_t len1 = strlen(currName1);
    size_t len2 = strlen(currName2);

    errno = 0;
    while ((entry = readdir(cd1)) != NULL) {

        currName1[len1] = 0;
        strcat(currName1, entry->d_name);
        currName2[len2] = 0;
        strcat(currName2, entry->d_name);

        if (lstat(currName1, &st1) == -1) {
            fprintf(stderr,"%s: %s\n", currName1, strerror(errno));
            continue; 
        }

        if (S_ISDIR(st1.st_mode)) {

            if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))  {

                // если нет такой папки, то создаю
                if (stat(currName2, &st2) != 0) {

                    if (mkdir(currName2, st1.st_mode) == -1){
                        fprintf(stderr,"%s: %s\n", currName2, strerror(errno));
                        continue; 
                    }
                }
                Process(currName1, currName2, maxCount);
            }
        }
        else if (S_ISREG(st1.st_mode))
        {
            if (count >= maxCount) {
                int status = 0;
                if (wait(&status) != -1) {
                    count--;
                }
            }

            if ((stat(currName2, &st2) != 0)) {
                pid_t pid = fork();
                if (pid == 0) {

                    int bytesCount = 0;
                    int errCode = CopyFile(currName1, currName2, &bytesCount);

                    if (errCode == -1) {
                        exit(-1);
                    }

                    if (chmod(currName2, st1.st_mode) == -1) {
                        fprintf(stderr,"%s: %s\n", currName2, strerror(errno));
                        exit(-1); 
                    }

                    printf("%d: %s (%d byte)\n", getpid(), currName1, bytesCount);
                    exit(EXIT_SUCCESS);
                }
                count++;
            }

        }
    }

    if (closedir(cd1) == -1){
        fprintf(stderr,"%s: %s\n", currName1, strerror(errno));
        return -1;
    }

    return 0;
}

int main(int argc, char **argv){

    if(argc != 4){
        fprintf(stderr, "Enter valid number of arguments: 1 - exe-file, 2 - directory.\n");
        return -1;
    }

    char *dir1 = realpath(argv[1], NULL);
    char *dir2 = realpath(argv[2], NULL);

    if(!dir1) {
        fprintf(stderr,"%s: %s: %s\n", argv[0], argv[1], strerror(errno));
        return -1;
    }
    else if(!dir2){
        fprintf(stderr,"%s: %s: %s\n", argv[0], argv[2], strerror(errno));
        return -1;
    }

    char* endptr;
    char* maxCount = argv[3];
    long val = strtol(maxCount, &endptr, 10);

    if (val == LONG_MAX || val == LONG_MIN){
        perror("strtol");
        return -1;
    }
    else if (val < 0){
        fprintf(stderr, "Number less than zero");
        return -1;
    }

    if (endptr == maxCount){
        fprintf(stderr, "No digits were found\n");
        return -1;
    }

    if (*endptr != '\0'){
        printf("Further characters after number: %s\n", endptr);
    }

    Process(dir1, dir2, val);

    while (1){
        if (wait(NULL) == -1){
            if (errno == ECHILD){
                break;
            }
        }
    }

    free(dir1);
    free(dir2);
    return 0;
}
