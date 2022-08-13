
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>          // permission flags
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>         // open/close
#include <sys/stat.h>       // check file info
#include <sys/types.h>
#include <sys/resource.h>
// #include <sys/file.h>       // flock


#define LOCKFILE "/var/run/ots_daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int lock_fd = -1;

int lockfile(int fd) {
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl); // flock(fd, LOCK_SH); or LOCK_EX
}

int unlockfile(int fd) {
    struct flock fl;
    fl.l_type = F_UNLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl); // flock(fd, LOCK_UN);
}

int lock_file(void) {
    char buf[16];
    lock_fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
    if (lock_fd < 0) {
        printf("Can't open %s: %s\n", LOCKFILE, strerror(errno));
        return -1;
    }
    if (lockfile(lock_fd) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            printf("Can't lock %s: %s\n", LOCKFILE, strerror(errno));
            close(lock_fd);
            return 1;
        }
        printf("Can't lock %s: %s\n", LOCKFILE, strerror(errno));
        return -1;
    }
    ftruncate(lock_fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(lock_fd, buf, strlen(buf) + 1);
    return 0;
}


void handle_SIGHUP(int sig) {

}

void daemonize() {

    umask(0);

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        printf("Can't receive biggest descriptor number\n");
    }

    pid_t pid = fork();
    if (pid < 0) {
        printf("Failed to make first fork\n");
    } else if (pid != 0) {
        printf("First fork pid: %d\n", pid);
        exit(0);
    }

    setsid();

    struct sigaction sa;
    sa.sa_handler = handle_SIGHUP;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        printf("Failed to block SIGHUP\n");
    }

    pid = fork();
    if (pid < 0) {
        printf("Failed to make second fork\n");
    } else if (pid != 0) {
        printf("Second fork pid: %d\n", pid);
        exit(0);
    }

    if (chdir("/") < 0) {
        perror("Failed to move to root\n");
    }

    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }

    for (long unsigned int i = 0; i < rl.rlim_max; i++) {
        close(i);
    }
    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        printf("Wrong file descriptors fd0 = %d, fd1 = %d, fd2 = %d\n", fd0, fd1, fd2);
    }
}

int main(int argc, char* argv[]) {
    if (lock_file() != 0) {
        return 0;
    }
    // config /etc/daemon-name.conf
    // Запуск через /etc/init.d/daemon-name, возможность запуска без демонизации
    daemonize();
    unlockfile(lock_fd);
    close(lock_fd);
    return 0;
}
