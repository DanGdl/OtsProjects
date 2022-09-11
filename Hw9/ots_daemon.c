
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
#include <sys/socket.h>
#include <stddef.h>
#include <sys/un.h>
#include <linux/limits.h>


#include "ots_daemon.h"


#define SIZE_BUFFER 128
#define LOCKFILE "/var/run/ots_daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)


char filename[PATH_MAX - 1] = { '\0' };
int lock_fd = -1;
int socket_descriptor = -1;
int has_interrupt = 0;


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
    sprintf(buf, "%ld", (long) getpid());
    write(lock_fd, buf, strlen(buf) + 1);
    return 0;
}


int read_config() {
    FILE* f = fopen(PATH_CONFIG, "r");
    if (f == NULL) {
        printf("Fail to read config\n");
        return -1;
    }
    fgets((char*) &filename, PATH_MAX - 1, f);
    // remove new lines, it leads to bugs
    char* position = NULL;
    do {
        position = strchr((char*) &filename, '\n');
        if (position != NULL) {
            *position = '\0';
        }
    } while(position != NULL);
    fclose(f);
    return 0;
}


int setup_socket() {
    if (read_config() < 0) {
        printf("Fail to read config\n");
        return -1;
    }

    // reuse socket
    unlink(NAME_SOCKET);

    socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_descriptor < 0) {
        printf("Fail to setup local socket\n");
        return -1;
    }

    struct sockaddr_un socket;
    memset(&socket, 0, sizeof(socket));
    socket.sun_family = AF_UNIX;
    strncpy(socket.sun_path, NAME_SOCKET, sizeof(socket.sun_path) - 1);

    const int size = offsetof(struct sockaddr_un, sun_path) + strlen(socket.sun_path);
    int ret = bind(socket_descriptor, (struct sockaddr *)&socket, size);
    if (ret < 0) {
//     if (bind(socket_descriptor, (struct sockaddr *)&socket, sizeof(socket)) < 0) {
        printf("Fail to bind local socket %s\n", strerror(errno));
        return -1;
    }

    ret = listen(socket_descriptor, 20);
    if (ret == -1) {
        printf("Fail to listen local socket\n");
        return -1;
    }

    char buffer[SIZE_BUFFER];
    for (;;) {
        const int data_socket = accept(socket_descriptor, NULL, NULL);
        if (data_socket == -1) {
            printf("Fail to accept local socket\n");
            return -1;
        }

        // get size of file and send it
        struct stat stats;
        stat(filename, &stats);
        sprintf(buffer, "%ld", stats.st_size);

        ret = write(data_socket, buffer, sizeof(buffer));
        if (ret == -1) {
            printf("Fail to write into local socket\n");
            return -1;
        }

        /* Close socket. */
        close(data_socket);
        if (has_interrupt) {
            break;
        }
    }

    if (socket_descriptor > 0) {
        close(socket_descriptor);
        socket_descriptor = -1;
        unlink(NAME_SOCKET);
    }

    return 0;
}


void handle_shutdown(int sig) {
    if (!has_interrupt) {
        has_interrupt = 1;
//         return;
    }
	if (socket_descriptor > 0) {
        close(socket_descriptor);
        unlink(NAME_SOCKET);
    }
    if (lock_fd > 0) {
		unlockfile(lock_fd);
        close(lock_fd);
	}
	exit(0);
}

/**
 * setup listener to receive signals
 */
int catch_signal(int sig, void (*handler)(int)) {
	struct sigaction action;
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	return sigaction(sig, &action, NULL);
}



void handle_SIGHUP(int sig) {
    if (read_config() < 0) {
        printf("Fail to read config\n");
    }
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

    if (catch_signal(SIGHUP, handle_SIGHUP)) {
        printf("Failed to set interrupt for SIGHUP\n");
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

    setup_socket();
}



int main(int argc, char* argv[]) {
    if (catch_signal(SIGINT, handle_shutdown) == -1) {
		printf("Fail to set interrupt handler\n");
	}
    if (lock_file() != 0) {
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], DAEMON_MODE) == 0) {
        daemonize();
    } else {
        setup_socket();
    }
    unlockfile(lock_fd);
    close(lock_fd);
    return 0;
}
