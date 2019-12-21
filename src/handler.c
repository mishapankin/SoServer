#include "handler.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "http.h"

int
read_file(const char *path, char *buff, int n)
{
    char full_path[300];
    snprintf(full_path, sizeof(full_path), "root/%s", path);
    int fd = open(full_path, O_RDONLY);
    if (fd == -1) {
        return -1;
    }

    int file_sz = read(fd, buff, n);
    close(fd);
    return file_sz;
}

void
handle_connection(int fd, int shm_id)
{
    int *array = shmat(shm_id, NULL, 0);

    struct HttpRequest req;

    while (http_recv(fd, &req) == 0) {
        puts(req.path);

        if (strcmp(req.path, "/num") == 0) {
            char num[30];
            snprintf(num, sizeof(num), "Number: %d", array[0]++);
            struct HttpResponse resp = {
                .code = HTTP_OK,
                .content_length = strlen(num),
                .content_type = HTTP_HTML,
            };
            http_resp(fd, resp, num, resp.content_length);
            continue;
        }

        char file[5000];
        int n = read_file(req.path, file, sizeof(file));

        if (n == -1) {
            n = read_file("404.html", file, sizeof(file));
        }
        struct HttpResponse resp = {
            .code = HTTP_OK,
            .content_type = HTTP_HTML,
            .content_length = n
        };
        if (strcmp(req.path, "/favicon.ico") == 0) {
            resp.content_type = HTTP_ICO;
        }
        http_resp(fd, resp, file, n);
    }
}