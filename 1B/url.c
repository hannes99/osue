#include "url.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int starts_with(char* s, char* with) {
    size_t start_len = strlen(with);
    if (strlen(s)<start_len) return 0;
    return strncmp(s, with, start_len) == 0;
}

int parse_url(char *data, url* parsed_url) {
    if(!starts_with(data, "http://")) {
        return 0;
    }
    size_t total_len = strlen(data);
    if(total_len == 7) {
        return -1;
    }
    parsed_url->path = "/";
    int i = 7;
    parsed_url->hostname = malloc(total_len-6);
    if(parsed_url->hostname == NULL) {
        fprintf(stderr, "server: Could not allocate memory for url hostname.");
        exit(EXIT_FAILURE);
    }
    char current = data[i];
    while (i<=total_len && current != ';' && current != '/' && current != ':' && current != '@' && current != '=' && current != '&') {
        parsed_url->hostname[i-7] = current;
        current = data[++i];
    }
    if(i == 7) {
        free(parsed_url->hostname);
        return -1;
    }
    parsed_url->hostname[i] = '\0';
    parsed_url->hostname = realloc(parsed_url->hostname, i-6);
    int hostname_end = i;
    if(hostname_end <= total_len) {
        parsed_url->path = malloc(total_len-hostname_end+1);
        if(parsed_url->path == NULL) {
            fprintf(stderr, "Could not allocate memory for url path.");
            exit(EXIT_FAILURE);
        }
        while (i<total_len) {
            parsed_url->path[i-hostname_end] = data[i];
            i++;
        }
        parsed_url->path[i-hostname_end] = '\0';
    }
    size_t filename_start = strlen(parsed_url->path);
    if(parsed_url->path[filename_start-1] != '/') {
        while(parsed_url->path[--filename_start] != '/');
        parsed_url->filename = malloc(i-hostname_end-filename_start-1);
        if(parsed_url->filename == NULL) {
            fprintf(stderr, "Could not allocate memory for url filename.");
            exit(EXIT_FAILURE);
        }
        strcpy(parsed_url->filename, parsed_url->path+filename_start+1);
    } else {
        parsed_url->no_file_specified = 1;
    }
    return 1;
}
