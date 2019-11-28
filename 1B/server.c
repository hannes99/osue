/**
 * @file server.c
 * @author Hannes Laimer <e11808227@student.tuwien.ac.at>
 * @date 28.10.2019
 *
 * @brief File contains an implementation of a http server.
 *
 * This is a simplified implementation of a HTTP server
 */

#include <netdb.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "http.h"

typedef struct {
    char* port;
    char* index;
    char* root_dir;
} arguments;

int interrupt_recieved = 0;
int listenfd;
int idle = 0;

void handle_signal(int sig) {
    if(idle) {
        close(listenfd);
        exit(EXIT_SUCCESS);
    }
    interrupt_recieved = 1;
}

/**
 * @brief prints the usage info
 */
void usage(void) {
    fprintf(stderr, "Usage: %s [-p PORT] [-i INDEX] DOC_ROOT\n", "server");
}

int parse_arguments(int argc, char **argv, arguments *ret) {
    ret->port = "8080";
    ret->index = "index.html";
    ret->root_dir = NULL;
    char* result;
    long port;
    char c;
    while ((c = getopt(argc, argv, "p:i:")) != -1) {
        switch (c) {
            case 'p':
                port = strtol(optarg, &result, 10);
                if(*result != '\0' || port < 0 || port > 65535) {
                    fprintf(stderr, "%s: The port has to be a number between 0 and 65535.\n", "server");
                    return 0;
                }
                ret->port = optarg;
                break;
            case 'i':
                ret->index = optarg;
                break;
            default:
                usage();
                return 0;
                break;
        }
    }
    ret->root_dir = argv[optind];
    if (!ret->root_dir) {
        fprintf(stderr, "%s: A root directory has to be specified.\n", argv[0]);
        usage();
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    arguments options;
    if(parse_arguments(argc, argv, &options) == 0) {
        exit(EXIT_FAILURE);
    }
    struct addrinfo hints, *res, *p;
    listenfd = -1;
    // getaddrinfo for host
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, options.port, &hints, &res) != 0) {
        perror ("getaddrinfo() error");
        exit(EXIT_FAILURE);
    }

    // socket and bind
    for (p = res; p!=NULL; p=p->ai_next) {
        int option = 1;
        listenfd = socket (p->ai_family, p->ai_socktype, p->ai_protocol);
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    freeaddrinfo(res);

    if (p==NULL) {
        perror ("socket() or bind()");
        exit(EXIT_FAILURE);
    }
    if ( listen (listenfd, 10) != 0 ){
        perror("listen() error");
        exit(EXIT_FAILURE);
    }


    int clientfd;
    FILE* client_connection_in;
    FILE* client_connection_out;
    FILE* source_file;
    struct sockaddr clientaddr;
    socklen_t addrlen;

    while(!interrupt_recieved) {
        fprintf(stdout, "WAITING\n");
        idle = 1;
        addrlen = sizeof(clientaddr);
        clientfd = accept (listenfd, &clientaddr, &addrlen);
        if(clientfd < 0) {
            fprintf(stderr, "Could not accept connection from %s", "bo");
            continue;
        }
        idle = 0;
        client_connection_in = fdopen(clientfd, "r");
        client_connection_out = fdopen(clientfd, "wb");
        if(client_connection_in == NULL) {
            fprintf(stderr, "Could not open in-connection with client!");
            continue;
        }
        if(client_connection_out == NULL) {
            fprintf(stderr, "Could not open out-connection with client!");
            fclose(client_connection_in);
            continue;
        }
        http_request req;
        parse_request(client_connection_in, &req);
        http_response resp;
        if(strncmp("GET", req.method, 3) != 0) {
            resp.status = 501;
            resp.status_msg = "Not implemented";
            resp.content = NULL;
            generate_header(&resp);
        } else {
            if(req.path[strlen(req.path)-1]== '/') {
                req.path = realloc(req.path, strlen(req.path)+strlen(options.index));
                strcat(req.path, options.index);
            }
            char* file_path = malloc(strlen(req.path)+strlen(options.root_dir));
            if(file_path == NULL) {
                fprintf(stderr, "server: Could not allocate memory for filepath.");
                exit(EXIT_FAILURE);
            }
            strcpy(file_path, options.root_dir);
            strcat(file_path, req.path);
            fprintf(stdout, "FPATH: %s\n", file_path),
            source_file = fopen(file_path, "rb+");
            free(file_path);
            if(source_file == NULL) {
                resp.status = 404;
                resp.status_msg = "Not found";
                resp.content = NULL;
                generate_header(&resp);
            } else {
                resp.status = 200;
                resp.status_msg = "OK";
                int content_size = 0;
                int buffer_size = 100;
                resp.content = malloc(buffer_size);
                if(resp.content == NULL) {
                    fprintf(stderr, "server: Could not allocate memory for response content.");
                    exit(EXIT_FAILURE);
                }
                while(!feof(source_file)) {
                    resp.content[content_size++] = fgetc(source_file);
                    if(content_size >= (buffer_size-1)) {
                        buffer_size+=100;
                        resp.content = realloc(resp.content, buffer_size);
                        if(resp.content == NULL) {
                            fprintf(stderr, "server: Could not reallocate memory for response content.");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                resp.content[content_size-1] = '\0';
                resp.content = realloc(resp.content, content_size);
                generate_header(&resp);
            }
        }

        if(fprintf(client_connection_out, "%s\r\n\r\n", resp.header) < 0) {
            perror("server: writing response header failed: ");
            continue;
        }
        if(resp.content != NULL) {
            if(fprintf(client_connection_out, "%s", resp.content) < 0) {
                perror("server: writing response content failed: ");
                continue;
            }
        }
        if(fflush(client_connection_out)!=0){
            perror("server: error during flush: ");
            continue;
        }
        fclose(client_connection_in);
        fclose(client_connection_out);
        free_http_request(&req);
        free_http_response(&resp);
    }
    close(listenfd);
    exit(EXIT_SUCCESS);
}
