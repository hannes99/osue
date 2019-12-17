//
// Created by hannesl on 14.12.19.
//

#include <stdio.h>

static char *p_name;

void init_logger(char *p){
    p_name = p;
}

void log_error(char *error) {
    fprintf(stderr, "%s: %s\n", p_name, error);
}
void log_perror(char *desc) {
    fprintf(stderr, "%s: ", p_name);
    perror(desc);
}
