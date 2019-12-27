#include "shared.h"
#include "logger.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

shm *initSMem(int *shmfd) {
    shm *ret;
    *shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);

    size_t memSize = sizeof(*ret);

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

    return ret;
}

void closeSMem(shm *mem, int fd) {
    if (munmap(mem, sizeof(*mem)) == -1) {
        log_error("Could not munmap smem!");
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1) {
        log_perror("Could not close smem fd!");
    }
}

void removeSMem(shm *mem, int fd) {
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