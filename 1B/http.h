#ifndef HTTP_H
#define HTTP_H

#include "url.h"
#include "http.h"

typedef struct {
    char* header;
    char* content;
    long int status;
    char* status_msg;
    int valid;
} http_response;

typedef struct {
    char* method;
    char* path;
    int valid;
} http_request;

/**
 * @brief performs a HTTP GET request to a server specified in url and saved the response in http_response
 */
void get_http(url*, http_response*);

/**
 * @brief parses a request which is read on a FILE* and saved it to a http_request
 */
void parse_request(FILE*, http_request*);

/**
 * @brief generates HTTP header for a given response
 *
 * The generated header is written back to the http_response struct
 */
void generate_header(http_response*);

/**
 * @brief frees memory
 */
void free_http_response(http_response*);

/**
 * @brief frees memory
 */
void free_http_request(http_request*);

#endif
