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
    char *file_name = "/home/houd4aif4/.temp/.hidden/.process/.log.txt";
    FILE* file;
    time_t now;
    char time_str[64];

    pid_t pid = fork();

    // an error occurred while creating the process
    if (pid < 0) {
        exit(EXIT_FAILURE);
        return -1;
    }

    // stop the parent process
    if (pid > 0) {
        _exit(EXIT_SUCCESS);
    }

    // // write into the a log file 
    // char status_file[60];
    // char output_file[] = "/home/houd4aif4/.temp/.hidden/.process/.status_output.txt";

    // sprintf(status_file, "/proc/%d/status", getpid());
    // FILE *status = fopen(status_file, "r");
    // if (status) {
    //     FILE *output = fopen(output_file, "w");
    //     if (output) {
    //         char line[256];
    //         while (fgets(line, sizeof(line), status)) {
    //             fprintf(output, "%s", line);
    //         }
    //         fclose(output);
    //     }
    //     fclose(status);
    // }

    // set the HIDE_PID env variable to the current running process PID
    FILE *temp;
    char *pid_file = "/home/houd4aif4/.temp/.hidden/.process/.pid.txt";

    temp = fopen(pid_file, "w");
    if (!temp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(temp, "%d", getpid());
    fclose(temp);

    // print the pcb info    
    struct statStuff s;
    if (show_pcb_info(getpid(), &s)) {
        char output_file[] = "/home/houd4aif4/.temp/.hidden/.process/.status_output.txt";
        FILE* output = fopen(output_file, "w");
        printStat(output, &s);
        fclose(output);
    }


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