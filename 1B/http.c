#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define ERROR_EXIT(...) { fprintf(stderr, "ERROR: " __VA_ARGS__); exit(EXIT_FAILURE); }

#include "http.h"

void generate_header(http_response *res) {
    char *date_buffer = malloc(200);
    if(date_buffer == NULL) {
        fprintf(stderr, "Could not allocate memory for header timestamp.");
        exit(EXIT_FAILURE);
    }
    long content_len = 0;
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    res->header = malloc(500);
    if(res->header == NULL) {
        fprintf(stderr, "Could not allocate memory for response header.");
        exit(EXIT_FAILURE);
    }
    strftime(date_buffer, 200, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    if(res->content != NULL) content_len = strlen(res->content);
    sprintf(res->header, "HTTP/1.1 %ld %s\r\n"
                         "Date: %s\r\n"
                         "Content-Length: %ld\r\n"
                         "Connection: close", res->status, res->status_msg, date_buffer, content_len);
    res->header = realloc(res->header, strlen(res->header));
    free(date_buffer);
}

void parse_request(FILE *in, http_request *req) {
    req->method = malloc(20);
    if(req->method == NULL) {
        fprintf(stderr, "Could not allocate memory for request method.");
        exit(EXIT_FAILURE);
    }
    req->valid = 1;
    int method_index = 0;
    char current = fgetc(in);
    while (!feof(in) && current != ' ') {
        req->method[method_index++] = current;
        current = fgetc(in);
    }
    req->method = realloc(req->method, method_index + 1);
    req->method[method_index] = '\0';

    int buffer_size = 10;
    req->path = malloc(buffer_size);
    if(req->path == NULL) {
        fprintf(stderr, "Could not allocate memory for request path.");
        exit(EXIT_FAILURE);
    }
    int path_index = 0;
    current = fgetc(in);
    while (!feof(in) && current != ' ') {
        req->path[path_index++] = current;
        current = fgetc(in);
        if (path_index >= (buffer_size - 1)) {
            buffer_size += 10;
            req->path = realloc(req->path, buffer_size);
        }
    }
    req->path[path_index++] = '\0';
    req->path = realloc(req->path, path_index);

    char *buffer = malloc(100);
    if(buffer == NULL) {
        fprintf(stderr, "Could not allocate memory for buffer to check for HTTP/1.1 in header.");
        exit(EXIT_FAILURE);
    }
    int pos = 0;
    while (!feof(in) && pos <= 7) {
        buffer[pos++] = fgetc(in);
    }

    if (strncmp("HTTP/1.1", buffer, 8) != 0 || path_index == 0 || pos == 0) {
        req->valid = 0;
        return;
    }

    // GZIP finden

}

void parse_http_response(FILE *sockfile, http_response *res) {

    res->valid=1;

    // check for HTTP/1.1 beginning
    char *buffer = malloc(100);
    if(buffer == NULL) {
        fprintf(stderr, "Could not allocate memory for buffer to check for HTTP/1.1 in header.");
        exit(EXIT_FAILURE);
    }
    int pos = 0;
    while (!feof(sockfile) && pos <= 7) {
        buffer[pos++] = fgetc(sockfile);
    }
    if (strncmp("HTTP/1.1 ", buffer, 8) != 0) {
        res->valid = 0;
        return;
    }

    // check for valid status code
    pos = 0;
    fgetc(sockfile);
    char current = fgetc(sockfile);
    while (!feof(sockfile) && current != ' ') {
        buffer[pos++] = current;
        current = fgetc(sockfile);
    }
    buffer[pos] = '\0';
    char *result;
    res->status = strtol(buffer, &result, 10);
    if (*result != '\0') {
        res->valid = 0;
        free(buffer);
        return;
    }

    // extract status message
    pos = 0;
    current = fgetc(sockfile);
    while (!feof(sockfile) && current != '\r') {
        buffer[pos++] = current;
        current = fgetc(sockfile);
    }
    buffer[pos] = '\0';
    res->status_msg = malloc(pos);
    if(res->status_msg == NULL) {
        fprintf(stderr, "Could not allocate memory for status message.");
        exit(EXIT_FAILURE);
    }
    strcpy(res->status_msg, buffer);

    // fetch content and header
    pos = 0;
    int mem_size = 100;
    res->content = malloc(mem_size);
    if(res->content == NULL) {
        fprintf(stderr, "Could not allocate memory for response content.");
        exit(EXIT_FAILURE);
    }
    while (!feof(sockfile)) {
        res->content[pos++] = fgetc(sockfile);
        if (pos >= (mem_size - 1)) {
            mem_size += 100;
            res->content = realloc(res->content, mem_size);
        }
    }
    res->content = realloc(res->content, pos);
    res->content[pos - 1] = '\0';
    // skip header
    while (!(res->content[0] == '\r' && res->content[1] == '\n' && res->content[2] == '\r' &&
             res->content[3] == '\n')) {
        res->content++;
    }
    res->content += 4;
    free(buffer);
}

void get_http(url *url_information, http_response *res) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        ERROR_EXIT("Could not create socket!")
    }

    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // create Internet Protocol (IP) socket
    hints.ai_socktype = SOCK_STREAM; // use TCP as transport protocol

    int addr_info = getaddrinfo(url_information->hostname, url_information->port, &hints, &ai);
    if (addr_info != 0) {
        fprintf(stderr, "HOST: %s\n", url_information->hostname);
        fprintf(stderr, "PORT: %s\n", url_information->port);
        ERROR_EXIT("getaddrinfo: %s\n", gai_strerror(addr_info));
    }

    if (connect(sockfd, ai->ai_addr, ai->ai_addrlen) < 0) ERROR_EXIT("connect: %s\n", strerror(errno));

    FILE *sockfile = fdopen(sockfd, "rwb+");
    if (sockfile == NULL) ERROR_EXIT("fdopen: %s\n", strerror(errno));

    fprintf(sockfile, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url_information->path,
            url_information->hostname);

    parse_http_response(sockfile, res);

    fclose(sockfile);
}

void free_http_response(http_response* resp) {
    free(resp->content);
    free(resp->header);
}

void free_http_request(http_request* req) {
    free(req->path);
    free(req->method);
}
