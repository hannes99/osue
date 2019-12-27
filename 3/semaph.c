//
// Created by hannesl on 27.12.19.
//

#include "semaph.h"

#include <fcntl.h>
#include <stdlib.h>
#include "logger.h"

sem_t *initSem(char *name, size_t initSize) {
    sem_t *ret = sem_open(name, O_CREAT, 0600, initSize);
    if (ret == SEM_FAILED) {
        log_perror("Could not create semaphore!");
        exit(EXIT_FAILURE);
    }
    return ret;
}

sem_t *openSem(char *name) {
    sem_t *ret = sem_open(name, O_EXCL);
    if (ret == SEM_FAILED) {
        log_perror("Could not open semaphore!");
        exit(EXIT_FAILURE);
    }
    return ret;
}

void removeSem(char *name) {
    if (sem_unlink(name) == -1) {
        log_perror("Could not unlink read semaphore!");
        exit(EXIT_FAILURE);
    }
}

void closeSem(sem_t *sem) {
    if (sem_close(sem) == -1) {
        log_perror("Could not close read semaphore!");
        exit(EXIT_FAILURE);
    }
}