#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "handler.h"

enum { MAX_CONNECTIONS_COUNT = 50 };

volatile int socket_fd;
volatile int connect_fd;
volatile int shm_id;

void
main_handle_sigint(int signum)
{
    while (wait(NULL) != -1) {}
    close(socket_fd);
    shmctl(shm_id, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}

void
child_handle_sigint(int signum)
{
    close(connect_fd);
    recv(connect_fd, NULL, 0, 0);
    exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        fputs("No port", stdout);
        exit(EXIT_FAILURE);
    }
    shm_id = shmget(IPC_PRIVATE, sizeof(int), 0666 | IPC_CREAT);

    int port;
    sscanf(argv[1], "%d", &port);

    socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1) {
        perror("Creating socket error");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt (socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) == -1) {
        perror("Socket option error");
    }

    struct sockaddr_in addr = {
        .sin_family = PF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };

    if (bind(socket_fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("Binding error");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    if (listen(socket_fd, MAX_CONNECTIONS_COUNT) == -1) {
        perror("Listening error");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    struct sigaction sa = {
        .sa_mask = sigset,
        .sa_handler = main_handle_sigint,
    };
    sigaction(SIGINT, &sa, NULL);

    while (1) {
        while (waitpid(0, NULL, WNOHANG) != -1) {}

        connect_fd = accept(socket_fd, NULL, NULL);
        sigprocmask(SIG_BLOCK, &sigset, NULL);
        if (connect_fd == -1) {
            break;
        }
        
        if (fork() == 0) {
            sa.sa_handler = child_handle_sigint;
            sigaction(SIGINT, &sa, NULL);
            sigprocmask(SIG_UNBLOCK, &sigset, NULL);

            handle_connection(connect_fd, shm_id);

            close(connect_fd);
            exit(EXIT_SUCCESS);
        }
        close(connect_fd);
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);
    }

    close(socket_fd);
    while (wait(NULL) != -1) {}

    return EXIT_SUCCESS;
}
