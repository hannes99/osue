/**
 * @file logger.c
 * @author Hannes Laimer <e11808227@student.tuwien.ac.at>
 * @date 20.12.2019
 *
 * @brief File contains an implementation of a logger.
 *
 * This is an implementation of a logger which prints error-messages to stderr
 */

#include <stdio.h>

static char *p_name;
static int p_pid;

/**
 * @brief Initializes the logger
 *
 * @param p name of the program
 * @param pid process-id of the program
 */
void init_logger(char *p, int pid){
    p_name = p;
    p_pid = pid;
}

/**
 * @brief writes a message to stderr
 *
 * Program name and PID are added in front of the message itself
 *
 * @param error the message
 */
void log_error(char *error) {
    fprintf(stderr, "%s(%d): %s\n", p_name, p_pid, error);
}

/**
 * @brief Calls perror() with program name and PID
 *
 * @param desc description of where the error occured
 */
void log_perror(char *desc) {
    fprintf(stderr, "%s(%d): ", p_name, p_pid);
    perror(desc);
}
