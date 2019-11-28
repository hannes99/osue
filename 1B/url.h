#ifndef URL_H
#define URL_H

typedef struct Url {
    char *hostname;
    char *port;
    char *path;
    char *filename;
    int no_file_specified;
} url;

/**
 * @brief parses a string and saved information into a url struct
 * @return
 */
int parse_url(char*, url*);

/**
 * brief checks if a geven string starts with an other string
 * @return
 */
int starts_with(char*, char*);

#endif
