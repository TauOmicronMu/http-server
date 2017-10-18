#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "http.h"

// Create a general headers struct with default values
struct general_headers *create_general_headers() {
    struct general_headers *hdr = calloc(1, sizeof(struct general_headers));
    
    hdr->date = NULL;
    hdr->connection = NULL;

    return hdr;
}

// Add the current timestamp to the general_headers struct, hdr
int general_headers_add_timestamp(struct general_headers *hdr) {
    if(!hdr) return -1;

    printf("add timestamp called\n");

    time_t curr_time = time(NULL);
    char *timestr = ctime(&curr_time); 

    hdr->date = calloc(1, strlen(timestr) * sizeof(char));
    strcpy(hdr->date, timestr);

    return 0;
}

// Add the given connection, conn, to the general_headers struct, hdr
int general_headers_add_connection(struct general_headers *hdr, char *conn) {
    if(!hdr) return -1;

    hdr->connection = calloc(1, strlen(conn) * sizeof(char));
    strcpy(hdr->connection, conn);

    return 0;
}

// Create a response_headers struct with default values
struct response_headers *create_response_headers() {
    struct response_headers *hdr = calloc(1, sizeof(struct response_headers));

    hdr->server = NULL;
    hdr->accept_ranges = NULL;

    return hdr;
}

// Add the given server, serv, to the response_headers struct, hdr
int response_headers_add_server(struct response_headers *hdr, char *serv) {
    if(!hdr) return -1;

    hdr->server = calloc(1, strlen(serv) * sizeof(char));
    strcpy(hdr->server, serv);

    return 0;
}

// Add the given accept_ranges, ars, to the response_headers struct, hdr
int response_headers_add_accept_ranges(struct response_headers *hdr, char *ars) {
    if(!hdr) return -1;

    hdr->accept_ranges = calloc(1, strlen(ars) * sizeof(char));
    strcpy(hdr->accept_ranges, ars);

    return 0;
}

// Create an entity_headers struct with default values
struct entity_headers *create_entity_headers() { 
    struct entity_headers *hdr = calloc(1, sizeof(struct entity_headers));

    hdr->content_type = NULL;
    hdr->content_length = 0;

    return hdr;
}

// Add the given content_type, type, to the entity_headers struct, hdr
int entity_headers_add_content_type(struct entity_headers *hdr, char *type) {
    if(!hdr) return -1;

    hdr->content_type = calloc(1, strlen(type) * sizeof(char));
    strcpy(hdr->content_type, type);

    return 0;
} 

// Create an http_response struct with default values
struct http_response *create_http_response() {
    struct http_response *res = calloc(1, sizeof(struct http_response));

    res->status = 0;
    res->general_headers = create_general_headers();
    res->response_headers = create_response_headers();
    res->entity_headers = create_entity_headers();
    res->body = calloc(1, sizeof(char *));
 
    return res;
}

// Free all memory in use by given general_headers, hdr
int destroy_general_headers(struct general_headers *hdr) {
    free(hdr->date);
    free(hdr->connection);
    free(hdr);

    return 0;
}

// Free all memory in use by given response_headers, hdr
int destroy_response_headers(struct response_headers *hdr) {
    free(hdr->server);
    free(hdr->accept_ranges);
    free(hdr);

    return 0;
}

// Free all memory in use by given entity_headers, hdr
int destroy_entity_headers(struct entity_headers *hdr) {
    free(hdr->content_type);
    free(hdr);

    return 0;
}

// Free all memory in use by a given http_response, res
int destroy_http_response(struct http_response *res) {    
    destroy_general_headers(res->general_headers);
    destroy_response_headers(res->response_headers);
    destroy_entity_headers(res->entity_headers);
    free(res);
  
    return 0;
}

// Adds a given body, body to an http_response, res.
int add_http_response_body(struct http_response *res, char *body) {
    if(!res->entity_headers) return -1;
    if(!res->entity_headers->content_length) return -1;
    if(res->body) return -1;
    
    // Reallocate enough space for the whole body and zero the new memory
    res->body = realloc(res->body, res->entity_headers->content_length);
    memset(res->body, 0, res->entity_headers->content_length * sizeof(char));

    // Copy the provided body into the response
    strcpy(res->body, body);

    return 0;
}

// Takes a raw http_response, res, and returns the equivalent http_response struct
struct http_response *parse_http_response(char *res) {
    // TODO: Implement this

    return 0;
}

// Converts an HTTP status code to its name
char *stat2nam(int stat) {
    switch(stat) {
        case 200 :
            return "OK";
            break;
        default :
            return NULL;
    }
}

// Sends an http_response, res, over the given socket, clisockfd
int send_http_response(int *clisockfd, struct http_response *res) {
    int n = fprintf(stdout, "HTTP/1.1 %d %s\n"
                                "Date: %s\n"
                                "Connection: %s\n" 
                                "Server: %s\n"
                                "Accept-Ranges: %s\n"
                                "Content-Type: %s\n"
                                "Content-Length: %d\n\n"
                                "%s",
                                res->status, 
                                stat2nam(res->status),
                                res->general_headers->date,
                                res->general_headers->connection,
                                res->response_headers->server,
                                res->response_headers->accept_ranges,
                                res->entity_headers->content_type,
                                res->entity_headers->content_length,
                                res->body);
    
    if(n < 0) return -1;
    return 0;
}

// Construct a full HTTP response (struct) from the required components
struct http_response *construct_http_response(int status, char *conn, char *serv, char *ars, char *type, int len, char *body) {
    // Create a blank http_response struct
    struct http_response *res = create_http_response();

    res->status = status;

    general_headers_add_connection(res->general_headers, conn);

    response_headers_add_server(res->response_headers, serv);

    response_headers_add_accept_ranges(res->response_headers, ars);

    res->entity_headers->content_length = len;

    entity_headers_add_content_type(res->entity_headers, type);

    add_http_response_body(res, body);

    // Timestamp the response
    general_headers_add_timestamp(res->general_headers);

    return res;
}

