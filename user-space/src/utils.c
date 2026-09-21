#include <stdio.h>
#include <stdlib.h>

// ----------------------------
// PCB structure (from /proc/[pid]/stat)
// ----------------------------
struct statStuff {
    int pid;
    char comm[256];
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned long flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    long cutime;
    long cstime;
    long priority;
    long nice;
    long num_threads;
    long itrealvalue;
    unsigned long starttime;
    unsigned long vsize;
    long rss;
    unsigned long rlim;
    unsigned long startcode;
    unsigned long endcode;
    unsigned long startstack;
    unsigned long kstkesp;
    unsigned long kstkeip;
    unsigned long signal;
    unsigned long blocked;
    unsigned long sigignore;
    unsigned long sigcatch;
    unsigned long wchan;
    unsigned long nswap;
    unsigned long cnswap;
    int exit_signal;
    int processor;
    unsigned long rt_priority;
    unsigned long policy;
    unsigned long long delayacct_blkio_ticks;
};

// ----------------------------
// Function to read PCB info from /proc/[pid]/stat
// ----------------------------
static int show_pcb_info(int pid, struct statStuff *s) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *proc = fopen(path, "r");
    if (!proc) {
        perror("Cannot open stat file");
        return 0;
    }
    const char *format =
        "%d (%255[^)]) %c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld "
        "%ld %lu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %lu %lu %llu";

    int n = fscanf(proc, format,
        &s->pid,
        s->comm,
        &s->state,
        &s->ppid,
        &s->pgrp,
        &s->session,
        &s->tty_nr,
        &s->tpgid,
        &s->flags,
        &s->minflt,
        &s->cminflt,
        &s->majflt,
        &s->cmajflt,
        &s->utime,
        &s->stime,
        &s->cutime,
        &s->cstime,
        &s->priority,
        &s->nice,
        &s->num_threads,
        &s->itrealvalue,
        &s->starttime,
        &s->vsize,
        &s->rss,
        &s->rlim,
        &s->startcode,
        &s->endcode,
        &s->startstack,
        &s->kstkesp,
        &s->kstkeip,
        &s->signal,
        &s->blocked,
        &s->sigignore,
        &s->sigcatch,
        &s->wchan,
        &s->nswap,
        &s->cnswap,
        &s->exit_signal,
        &s->processor,
        &s->rt_priority,
        &s->policy,
        &s->delayacct_blkio_ticks
    );

    fclose(proc);
    return n > 0;
}

static void printStat(FILE *out, struct statStuff *s) {
    fprintf(out, "| ----------- PCB (STAT) INFORMATION ----------|\n");
    fprintf(out, "| PID: %-40d|\n", s->pid);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| Command: %-36s|\n", s->comm);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| State: %-38c|\n", s->state);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| Parent PID: %-32d |\n", s->ppid);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| Process Group: %-28d  |\n", s->pgrp);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| Session ID: %-32d |\n", s->session);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| Priority: %ld                                 |\n", s->priority);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| TTY: %-40d|\n", s->tty_nr);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| User Time (jiffies): %-24lu|\n", s->utime);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| System Time (jiffies): %-22lu|\n", s->stime);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| VirtualMemorySize: %-20lu bytes|\n", s->vsize);
    fprintf(out, "|----------------------------------------------|\n");
    fprintf(out, "| Resident Set Size: %-21ldpages|\n", s->rss);
    fprintf(out, "|----------------------------------------------|\n");
}


// int main() {
//     char path[256];
//     const char *home = getenv("HOME");
//     snprintf(path, sizeof(path), "%s/.temp/.hidden/.process/.pid.txt", home);

//     FILE *pid_file = fopen(path , "r");
//     if (!pid_file) {
//         perror("PID file not found");
//         return 1;
//     }

//     int pid;
//     fscanf(pid_file, "%d", &pid);
//     fclose(pid_file);

//     struct statStuff s;

//     if (show_pcb_info(pid, &s)) {
//         printStat(stdout, &s);
//     } else {
//         printf("Failed to read PCB info for PID %d\n", pid);
//     }

//     return 0;
// }
