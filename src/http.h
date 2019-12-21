#ifndef _HTTP_H
#define _HTTP_H

enum HttpType
{
    HTTP_GET,
    HTTP_POST,
    HTTP_OTHER,
};

enum HttpContentType
{
    HTTP_HTML,
    HTTP_ICO,
};

enum HttpStatusCode
{
    HTTP_OK = 200,
    HTTP_NOT_FOUND = 404,
};

enum { HTTP_PATH_LEN = 300 };

struct HttpRequest
{
    enum HttpType type;
    char path[HTTP_PATH_LEN];
};

struct HttpResponse
{
    enum HttpStatusCode code;
    enum HttpContentType content_type;
    int content_length;
};

int http_recv(int fd, struct HttpRequest *req);
int http_resp(int fd, struct HttpResponse resp, char *content, int n);

int http_print_status(char *target, int n, enum HttpStatusCode code);
int http_print_content_type(char *target, int n, enum HttpContentType type);
int http_print_header(char *target, int n, struct HttpResponse resp);

#endif