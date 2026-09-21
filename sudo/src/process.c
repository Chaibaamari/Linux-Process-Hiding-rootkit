#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include "utils.c"

int main(int argc, char **argv) {
    pid_t pid = fork();

    // an error occurred while creating the process
    if (pid < 0) exit(EXIT_FAILURE);

    // stop the parent process
    if (pid > 0) exit(0);

    // daemonize the child process
    if (setsid() < 0) _exit(EXIT_SUCCESS);
    umask(0);
    chdir("/");

    // Get the pid and store it inside a file
    FILE *temp;
    char *pid_file = "/run/process.pid";

    // write the process pid into a file
    temp = fopen(pid_file, "w");
    if (!temp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(temp, "%d\n", getpid());
    fclose(temp);

    // print the pcb info    
    struct statStuff s;
    if (show_pcb_info(getpid(), &s)) {
        char output_file[] = "/opt/process/.status_output.txt";
        FILE* output = fopen(output_file, "w");
        printStat(output, &s);
        fclose(output);
    }

    char *file_name = "/var/log/process.log";
    FILE* file;
    time_t now;
    char time_str[64];


    // write into a file
    while (1) {
        // open the file in append mode
        file = fopen(file_name, "a");
        if (!file) {
            perror("fopen");
            break;
        }

        // get the current time and format it
        time(&now);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

        // log into the file the data and then close it
        fprintf(file, "[%s] process (PID: %d) is running\n", time_str, getpid());
        fclose(file);
        
        sleep(60);
    }
    
    return 0;
}