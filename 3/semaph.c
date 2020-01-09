/**
 * @file semaph.c
 * @author Hannes Laimer <e11808227@student.tuwien.ac.at>
 * @date 23.12.2019
 *
 * @brief File contains an implementation of utility functions for semaphores.
 *
 * These are implementations of utility functions for semaphores.
 */

#include "semaph.h"

#include <fcntl.h>
#include <stdlib.h>
#include "logger.h"

/**
 * @brief Initialises a new semaphore with a given name and a given initial value.
 *
 * @param name of the semaphore
 * @param initSize initial value of the semaphore
 * @return pointer to the created semaphore
 */
sem_t *initSem(char *name, size_t initSize) {
    sem_t *ret = sem_open(name, O_CREAT, 0600, initSize);
    if (ret == SEM_FAILED) {
        log_perror("Could not create semaphore!");
        exit(EXIT_FAILURE);
    }
    return ret;
}

/**
 * @brief Opens an existing semaphore with a given name.
 *
 * @param name of the semaphore
 * @return pointer to the opened semaphore
 */
sem_t *openSem(char *name) {
    sem_t *ret = sem_open(name, O_EXCL);
    if (ret == SEM_FAILED) {
        log_perror("Could not open semaphore!");
        exit(EXIT_FAILURE);
    }
    return ret;
}

/**
 * @brief Removes an existing semaphore with a given name.
 *
 * @param name of the semaphore
 */
void removeSem(char *name) {
    if (sem_unlink(name) == -1) {
        log_perror("Could not unlink read semaphore!");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Closes an existing semaphore with a given name.
 *
 * @param name of the semaphore
 */
void closeSem(sem_t *sem) {
    if (sem_close(sem) == -1) {
        log_perror("Could not close read semaphore!");
        exit(EXIT_FAILURE);
    }
}