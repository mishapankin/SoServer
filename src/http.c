#include "http.h"

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

int
http_recv(int fd, struct HttpRequest *req)
{
    enum { MAX_REQ_SIZE = 1000 };
    char request[MAX_REQ_SIZE];
    int received = recv(fd, request, sizeof(request), 0);
    if (!req || received <= 0) {
        return -1;
    }
    enum { MAX_TYPE_LEN = 10 };
    char type[MAX_TYPE_LEN];
    if (sscanf(request, "%s %s", type, req->path) != 2) {
        return -1;
    }
    if (strcmp("GET", type) == 0) {
        req->type = HTTP_GET;
    } else if (strcmp("POST", type) == 0) {
        req->type = HTTP_POST;
    } else {
        req->type = HTTP_OTHER;
    }
    return 0;
}

int
http_print_status(char *target, int n, enum HttpStatusCode code)
{
    const char format_str[] = "HTTP/1.1 %d %s\r\n";

    if (code == HTTP_OK) {
        return snprintf(target, n, format_str, code, "OK");
    }
    if (code == HTTP_NOT_FOUND) {
        return snprintf(target, n, format_str, code, "Not found");
    }
    if (code == HTTP_FORBIDDEN) {
        return snprintf(target, n, format_str, code, "Forbidden");
    }
    return -1;
}

int
http_print_content_type(char *target, int n, enum HttpContentType type)
{
    if (type == HTTP_HTML) {
        return snprintf(target, n, "Content-Type: text/html; charset=UTF-8\r\n");
    }
    if (type == HTTP_ICO) {
        return snprintf(target, n, "Content-Type: image/x-icon\r\n");
    }
    return -1;
}

int
http_print_header(char *target, int n, struct HttpResponseHeader resp)
{
    int pos = 0;
    pos += http_print_status(target + pos, n - pos, resp.code);
    pos += http_print_content_type(target + pos, n - pos, resp.content_type);
    pos += snprintf(target + pos, n - pos, "Content-Length: %d\r\n", resp.content_length);
    pos += snprintf(target + pos, n - pos, "Server: My server\r\n");
    pos += snprintf(target + pos, n - pos, "Accept-Ranges: bytes\r\n");
    pos += snprintf(target + pos, n - pos, "Access-Control-Allow-Origin: *\r\n");
    pos += snprintf(target + pos, n - pos, "Connection: close\r\n");
    pos += snprintf(target + pos, n - pos, "\r\n");
    return pos;
}

int
http_send_header(int sock_fd, struct HttpResponseHeader resp)
{
    enum { BUFF_SIZE = 1000 };
    char buff[BUFF_SIZE];
    int ln = http_print_header(buff, sizeof(buff), resp);
    return send(sock_fd, buff, ln, 0);
}

int
http_send_content(int sock_fd, char *content, int n)
{
    int pos = 0;
    while (pos < n) {
        pos += send(sock_fd, content, n - pos, 0);
    }
    return 0;
}

int
http_send_file(int sock_fd, int fd)
{
    enum { BUFF_SIZE = 1000 };
    char buff[BUFF_SIZE];
    while (read(fd, buff, sizeof(buff)) > 0) {
        http_send_content(sock_fd, buff, sizeof(buff));
    }
    return 0;
}