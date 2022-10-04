#define _GNU_SOURCE
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EPOLL_EVENTS 128
#define BACKLOG 128

typedef struct ServerFds {
    int efd;
    int listenfd;
    int events_count;
    char* path;
}ServerFds_t;

static struct epoll_event events[MAX_EPOLL_EVENTS];
int can_handle = 1;


int setnonblocking(int sock) {
    int opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        fprintf(stderr, "Failed to setup nonblocking F_GETFL: %s\n", strerror(errno));
        return -1;
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0) {
        fprintf(stderr, "Failed to setup nonblocking F_SETFL: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}



int handle_server_connection(const ServerFds_t* const fds, struct epoll_event* connev) {
    int connfd = accept(fds -> listenfd, NULL, NULL);
    if (connfd < 0) {
        fprintf(stderr, "Failed to accept connection: %s\n", strerror(errno));
        return -1;
    }
    if (fds -> events_count == MAX_EPOLL_EVENTS - 1) {
        printf("Event array is full\n");
        close(connfd);
        return -1;
    }
    setnonblocking(connfd);
    connev -> data.fd = connfd;
    connev -> events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP;
    if (epoll_ctl(fds -> efd, EPOLL_CTL_ADD, connfd, connev) < 0) {
        fprintf(stderr, "Failed setup epoll: %s\n", strerror(errno));
        close(connfd);
        return -1;
    }
    return 0;
}


void handle_client_connection(const ServerFds_t* const fds, int fd, const struct epoll_event* const event) {
    if (events -> events & EPOLLRDHUP) {
        fprintf(stderr, "Error on fd %d\n", fd);
        return;
    }
    char buffer[512] = { '\0' };
    if (event -> events & EPOLLIN) {
        int rc = recv(fd, buffer, sizeof(buffer), 0);
        if (rc < 0) {
            fprintf(stderr, "Failed to read from fd: %s\n", strerror(errno));
            return;
        }
        fprintf(stderr, "Received %d\n", rc);
        buffer[rc] = '\0';
    }
    if (buffer[0] != '\0' && event -> events & EPOLLOUT) {
        char* file_path = NULL;
        int len = strlen(fds -> path);
		file_path = calloc(sizeof(*file_path), strlen(fds -> path) + len + 2);
		if (*(fds -> path + (len - 1)) == '/') {
			sprintf(file_path, "%s%s", fds -> path, buffer);
		} else {
			sprintf(file_path, "%s/%s", fds -> path, buffer);
		}

		// TODO: fix later
        char* result = NULL;
        struct stat stats;
        if (stat(file_path, &stats) != 0) {
            printf("Can't get state for path %s\n", file_path);

            result = "Can't get state for path\n";
        } else if (!S_ISREG(stats.st_mode)) {
            printf("File is not a regular file %s\n", file_path);

            result = "File is not a regular file\n";
        } else {
            result = "O hai!\n";
        }
        int rc = send(fd, result, strlen(result), 0);
        if (rc < 0) {
            fprintf(stderr, "Failed to write to fd: %s\n", strerror(errno));
        }
        free(file_path);
    }
}

ServerFds_t setup_epoll(int port) {
    ServerFds_t fds = {0};

    // fds.efd = epoll_create(MAX_EPOLL_EVENTS);
    fds.efd = epoll_create1(EPOLL_CLOEXEC);
    fds.listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (fds.listenfd < 0) {
        fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
        exit(0);
    }
    int reuse = 1;
	if (setsockopt(fds.listenfd, SOL_SOCKET, SO_REUSEADDR, (char*) &reuse, sizeof(int)) == 1) {
		fprintf(stderr, "Failed to reuse socket: %s\n", strerror(errno));
        exit(0);
	}
    setnonblocking(fds.listenfd);


    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    if (bind(fds.listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "Failed to bind socket: %s\n", strerror(errno));
        exit(0);
    }
    if (listen(fds.listenfd, BACKLOG) < 0) {
        fprintf(stderr, "Failed to listen socket: %s\n", strerror(errno));
        exit(0);
    }


    struct epoll_event listenev;
    listenev.events = EPOLLIN | EPOLLET;
    listenev.data.fd = fds.listenfd;
    if (epoll_ctl(fds.efd, EPOLL_CTL_ADD, fds.listenfd, &listenev) < 0){
        fprintf(stderr, "Failed add epoll fd: %s\n", strerror(errno));
        exit(0);
    }
    return fds;
}


void handle_shutdown(int sig) {
    can_handle = 0;
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


int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <port> <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char* path = argv[2];
    char* p;
    int port = strtol(argv[1], &p, 10);
    if (*p) {
        printf("Invalid port number\n");
        return EXIT_FAILURE;
    }
    struct stat stats;
    if (stat(path, &stats) != 0) {
        printf("Can't get state for path %s\n", path);
        return 0;
    } else if (!S_ISDIR(stats.st_mode)) {
        printf("Path %s is not a directory\n", path);
        return 0;
    }
    if (catch_signal(SIGINT, handle_shutdown) == -1) {
		printf("Fail to set interrupt handler\n");
        return 0;
	}

    signal(SIGPIPE, SIG_IGN);

    ServerFds_t fds = setup_epoll(port);
    fds.events_count = 1;
    fds.path = path;

    struct epoll_event connev;
    while (can_handle) {
        int nfds = epoll_wait(fds.efd, events, MAX_EPOLL_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == fds.listenfd && handle_server_connection(&fds, &connev) == 0) {
                fds.events_count++;
            } else {
                int fd = events[i].data.fd;
                handle_client_connection(&fds, fd, events + i);
                epoll_ctl(fds.efd, EPOLL_CTL_DEL, fd, &connev);
                close(fd);
                fds.events_count--;
            }
        }
    }
    close(fds.listenfd);
    close(fds.efd);
    return 0;
}
