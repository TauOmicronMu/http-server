#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"

// Create a general headers struct with default values
struct general_headers *create_general_headers() {
    struct general_headers *hdr = calloc(1, sizeof(struct general_headers));
    
    hdr->date = calloc(1, sizeof(char *));
    hdr->connection = calloc(1, sizeof(char *));

    return hdr;
}

// Create a response_headers struct with default values
struct response_headers *create_response_headers() {
    struct response_headers *hdr = calloc(1, sizeof(struct response_headers));

    hdr->server = calloc(1, sizeof(char *));
    hdr->accept_ranges = (1, sizeof(char *));

    return hdr;
}

// Create an entity_headers struct with default values
struct entity_headers *create_entity_headers() { 
    struct entity_headers *hdr = calloc(1, sizeof(struct entity_headers));

    hdr->content_type = calloc(1, sizeof(char *));
    hdr->content_length = 0;

    return hdr;
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
    realloc(res->body, res->entity_headers->content_length);
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

// Sends an http_response, res, over the given socket, clisockfd
int send_http_response(int *clisockfd, struct http_response *res) {
    // TODO: implement this

    return 0;
}
