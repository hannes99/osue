//
// Created by hannesl on 14.12.19.
//

#ifndef OSUE_LOGGER_H
#define OSUE_LOGGER_H

void init_logger(char *p_name, int pid);

void log_error(char *error);

void log_perror(char *desc);

#endif //OSUE_LOGGER_H
