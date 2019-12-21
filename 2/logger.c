//
// Created by hannesl on 14.12.19.
//

#include <stdio.h>

static char *p_name;
static int p_pid;

void init_logger(char *p, int pid){
    p_name = p;
    p_pid = pid;
}

void log_error(char *error) {
    fprintf(stderr, "%s(%d): %s\n", p_name, p_pid, error);
}
void log_perror(char *desc) {
    fprintf(stderr, "%s(%d): ", p_name, p_pid);
    perror(desc);
}
