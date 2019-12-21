//
// Created by hannesl on 14.12.19.
//

#include <stdio.h>
#include <unistd.h>

static char *p_name;

void init_logger(char *p){
    p_name = p;
}

void log_error(char *error) {
    fprintf(stderr, "%s(%d): %s\n", p_name, getpid(), error);
}
void log_perror(char *desc) {
    fprintf(stderr, "%s(%d): ", p_name, getpid());
    perror(desc);
}
