/**
 * @file client.c
 * @author Hannes Laimer <e11808227@student.tuwien.ac.at>
 * @date 28.10.2019
 *
 * @brief File contains an implementation of a http client.
 *
 * This is a simplified implementation of a HTTP client
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "url.h"
#include "http.h"


typedef struct {
    char *port;
    char *out_file;
    char *out_dir;
    char *url;
} arguments;


/**
 * @brief prints the usage info
 */
void usage(void) {
    fprintf(stderr, "Usage: %s [-p PORT] [ -o FILE | -d DIR ] URL\n", "client");
}

/**
 * Parses the passed commandline arguments
 *
 * @param argc argument count
 * @param argv the arguments themself
 * @param ret struct where the information will be saved
 * @return
 */
int parse_arguments(int argc, char **argv, arguments *ret) {
    ret->port = "80";
    ret->out_file = NULL;
    ret->out_dir = NULL;
    ret->url = NULL;
    char c;
    while ((c = getopt(argc, argv, "p:o:d:")) != -1) {
        switch (c) {
            case 'p':
                ret->port = optarg;
                break;
            case 'o':
                ret->out_file = optarg;
                break;
            case 'd':
                ret->out_dir = malloc(strlen(optarg));
                if(ret->out_dir == NULL) {
                    fprintf(stderr, "client: Could not allocate memory for output directory.");
                    exit(EXIT_FAILURE);
                }
                strcpy(ret->out_dir, optarg);
                int len = strlen(ret->out_dir);
                if(ret->out_dir[len-1] != '/') {
                    ret->out_dir = realloc(ret->out_dir, len+1);
                    ret->out_dir[len] = '/';
                    ret->out_dir[len+1] = '\0';
                }
                break;
            default:
                usage();
                return EXIT_FAILURE;
                break;
        }
    }
    ret->url = argv[optind];
    if (ret->out_dir && ret->out_file) {
        fprintf(stderr, "%s: Only a output directory or a output file can be specified, not both.\n", argv[0]);
        usage();
        return 0;
    }
    if (!ret->url) {
        fprintf(stderr, "%s: An url has to be specified.\n", argv[0]);
        usage();
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    arguments options;
    if (!parse_arguments(argc, argv, &options)) {
        exit(EXIT_FAILURE);
    }

    url parsed_url;
    parsed_url.port = options.port;
    int parse_result = parse_url(options.url, &parsed_url);
    if (parse_result < 1) {
        switch (parse_result) {
            case 0:
                fprintf(stderr, "%s: The passed URL does not start with \"http://\".\n", "client");
                break;
            case -1:
                fprintf(stderr, "%s: The URL is no valid, since the hostname is missing.\n", "client");
                break;
            default:
                fprintf(stderr, "%s: The URL could not be parsed. Make sure it is valid\n", "client");
        }
        usage();
        exit(EXIT_FAILURE);
    }

    http_response res;
    get_http(&parsed_url, &res);

    if (res.valid == 0) {
        fprintf(stderr, "Protocol error!\n");
        exit(EXIT_FAILURE);
    }
    if (res.status != 200) {
        fprintf(stderr, "%ld %s\n", res.status, res.status_msg);
        exit(3);
    } else {
        FILE* out = stdout;
        int close = 0;
        if (options.out_file) {
            close=1;
            out = fopen(options.out_file, "w+");
        } else if(options.out_dir) {
            close=1;
            char* path;
            if(parsed_url.no_file_specified == 1) {
                path = malloc(strlen(options.out_dir)+11);
            } else {
                path = malloc(strlen(options.out_dir)+strlen(parsed_url.filename));
            }
            if(path == NULL) {
                fprintf(stderr, "client: Could not allocate memory for output filepath.");
                exit(EXIT_FAILURE);
            }
            strcpy(path, options.out_dir);
            if(parsed_url.no_file_specified == 1) {
                strcat(path, "index.html");
            } else {
                strcat(path, parsed_url.filename);
            }
            out = fopen(path, "w+");
            free(path);
        }
        if(!out) {
            fprintf(stderr, "%s: File could not be opened", "client");
            exit(EXIT_FAILURE);
        }
        fprintf(out, "%s", res.content);
        if(close) fclose(out);
    }

    return EXIT_SUCCESS;
}
