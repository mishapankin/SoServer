#include "handler.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "http.h"

int
open_root(const char *root, const char *path)
{
    char *buff = malloc(strlen(path) + strlen(root) + 1);
    if (!buff) {
        return -1;
    }
    strcpy(buff, root);
    strcat(buff, path);
    puts(buff);
    int fd = open(buff, O_RDONLY);
    free(buff);
    return fd;
}

int
response_not_found(int sock_fd, const char *root)
{
    int fd = open_root(root, "404.html");
    struct stat st;
    fstat(fd, &st);
    struct HttpResponseHeader header = {
        .code = HTTP_NOT_FOUND,
        .content_type = HTTP_HTML,
        .content_length = st.st_size,
    };
    http_send_header(sock_fd, header);
    http_send_file(sock_fd, fd);
    close(fd);
    return 0;
}

int
response_forbidden(int sock_fd, const char *root)
{
    int fd = open_root(root, "403.html");
    struct stat st;
    fstat(fd, &st);
    struct HttpResponseHeader header = {
        .code = HTTP_FORBIDDEN,
        .content_type = HTTP_HTML,
        .content_length = st.st_size,
    };
    http_send_header(sock_fd, header);
    http_send_file(sock_fd, fd);
    close(fd);
    return 0;
}

int
response_file(int sock_fd, const char *path, enum HttpContentType type, const char *root)
{
    int fd = open_root(root, path);
    if (fd == -1) {
        return -1;
    }
    struct stat st;
    fstat(fd, &st);
    if (S_ISDIR(st.st_mode)) {
        return -1;
    }
    struct HttpResponseHeader header = {
        .code = HTTP_OK,
        .content_type = type,
        .content_length = st.st_size,
    };
    http_send_header(sock_fd, header);
    http_send_file(sock_fd, fd);
    close(fd);
    return 0;
}

void
handle_connection(int sock_fd, int shm_id)
{
    int *array = shmat(shm_id, NULL, 0);

    struct HttpRequest req;

    while (http_recv(sock_fd, &req) == 0) {
        puts(req.path);

        if (strcmp(req.path, "/num") == 0) {
            char num[30];
            snprintf(num, sizeof(num), "Number: %d", array[0]++);
            struct HttpResponseHeader resp = {
                .code = HTTP_OK,
                .content_length = strlen(num),
                .content_type = HTTP_HTML,
            };
            http_send_header(sock_fd, resp);
            http_send_content(sock_fd, num, resp.content_length);
            continue;
        }

        if (strstr(req.path, "../") != NULL) {
            response_forbidden(sock_fd, "root/");
            continue;
        }
        if (strcmp(req.path, "/favicon.ico") == 0) {
            response_file(sock_fd, "favicon.ico", HTTP_ICO, "root/");
            continue;
        }
        if (strcmp(req.path, "/") == 0) {
            response_file(sock_fd, "index.html", HTTP_HTML, "root/");
            continue;
        }
        if (response_file(sock_fd, req.path, HTTP_HTML, "root/") == -1) {
            response_not_found(sock_fd, "root/");
        }

    }
}