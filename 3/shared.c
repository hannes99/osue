#include "shared.h"
#include "logger.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

shm *initSMem(int *shmfd) {
    shm *ret;
    *shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);

    size_t memSize = 3 * sizeof(sem_t) + BUFFER_SIZE * sizeof(limitedEdgeSet);

    if (*shmfd == -1) {
        log_error("Could not open shared memory!");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(*shmfd, memSize) < 0) {
        log_error("Could not truncate shm!");
        exit(EXIT_FAILURE);
    }

    ret = mmap(NULL, memSize, PROT_READ | PROT_WRITE, MAP_SHARED, *shmfd, 0);

    if (ret == MAP_FAILED) {
        log_error("mmap failed!");
        exit(EXIT_FAILURE);
    }

    sem_t *x = sem_open(READ_SEM, O_CREAT, 0600, 0);
    if (x == SEM_FAILED) {
        log_perror("Could not create read semaphore!");
        exit(EXIT_FAILURE);
    }
    ret->data.readSem = *x;

    x = sem_open(WRITE_SEM, O_CREAT, 0600, 1);
    if (x == SEM_FAILED) {
        log_perror("Could not create write semaphore!");
        exit(EXIT_FAILURE);
    }
    ret->data.writeSem = *x;

    x = sem_open(BUFFER_FULL_SEM, O_CREAT, 0600, BUFFER_SIZE);
    if (x == SEM_FAILED) {
        log_perror("Could not create buffer full semaphore!");
        exit(EXIT_FAILURE);
    }
    ret->data.bufferFullSem = *x;

    ret->done = 0;
    return ret;
}

shm *openSMem(int *shmfd) {
    shm *ret;
    *shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (*shmfd == -1) {
        log_error("Could not open shared memory!");
        exit(EXIT_FAILURE);
    }

    ret = mmap(NULL, sizeof(*ret), PROT_READ | PROT_WRITE, MAP_SHARED, *shmfd, 0);

    if (ret == MAP_FAILED) {
        log_error("mmap failed!");
        exit(EXIT_FAILURE);
    }

    sem_t *x = sem_open(READ_SEM, O_EXCL);
    if (x == SEM_FAILED) {
        log_perror("Could not open read semaphore!");
        exit(EXIT_FAILURE);
    }
    ret->data.readSem = *x;

    x = sem_open(WRITE_SEM, O_EXCL);
    if (x == SEM_FAILED) {
        log_perror("Could not open write semaphore!");
        exit(EXIT_FAILURE);
    }
    ret->data.writeSem = *x;

    x = sem_open(BUFFER_FULL_SEM, O_EXCL);
    if (x == SEM_FAILED) {
        log_perror("Could not open buffer full semaphore!");
        exit(EXIT_FAILURE);
    }
    ret->data.bufferFullSem = *x;

    return ret;
}

void closeSMem(shm *mem, int fd) {
    if (sem_close(&mem->data.readSem) == -1) {
        log_perror("Could not close read semaphore!");
        exit(EXIT_FAILURE);
    }

    if (sem_close(&mem->data.writeSem) == -1) {
        log_perror("Could not close write semaphore!");
        exit(EXIT_FAILURE);
    }

    if (sem_close(&mem->data.bufferFullSem) == -1) {
        log_perror("Could not close buffer full semaphore!");
        exit(EXIT_FAILURE);
    }

    if (munmap(mem, sizeof(*mem)) == -1) {
        log_error("Could not munmap smem!");
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1) {
        log_perror("Could not close smem fd!");
    }
}

void removeSMem(shm *mem, int fd) {

    if (sem_unlink(READ_SEM) == -1) {
        log_perror("Could not unlink read semaphore!");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(WRITE_SEM) == -1) {
        log_perror("Could not unlink write semaphore!");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(BUFFER_FULL_SEM) == -1) {
        log_perror("Could not unlink write semaphore!");
        exit(EXIT_FAILURE);
    }

    if (munmap(mem, sizeof(*mem)) == -1) {
        log_error("Could not munmap smem!");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(SHM_NAME) == -1) {
        log_error("Could not unlink smem!");
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1) {
        log_perror("Could not close smem fd!");
    }
}

void addEdgeSet(shm *mem, edgeSet e) {

}