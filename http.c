#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "http.h"

/* ========================= HTTP REQUEST =========================  */

struct request_line *request_line_create() {
    struct request_line *reql = calloc(1, sizeof(struct request_line));

    reql->http_verb = NULL;
    reql->request_uri = NULL;

    return reql;
}

int request_line_add_http_verb(struct request_line *reql, char *verb) {
    if(!reql) return -1;

    int len = strlen(verb) + 1;    
    reql->http_verb = calloc(1, len * sizeof(char));
    strcpy(reql->http_verb, verb);

    return 0;
}

int request_line_add_uri(struct request_line *reql, char *uri) {
    if(!reql) return -1;

    int len = strlen(uri) + 1;
    reql->request_uri = calloc(1, len * sizeof(char));
    strcpy(reql->request_uri, uri);

    return 0;
}

int request_line_destroy(struct request_line *reql) {
    free(reql->http_verb);
    free(reql->request_uri);
    free(reql);

    return 0;
}

struct request_headers *request_headers_create() {
    struct request_headers *reqh = calloc(1, sizeof(struct request_headers));

    reqh->host = NULL;
    reqh->accept_language = NULL;
    reqh->user_agent = NULL;
    reqh->content_length = 0;

    return reqh;
}


int request_headers_add_host(struct request_headers *reqh, char *host) {
    if(!reqh) return -1;

    int len = strlen(host) + 1;
    reqh->host = calloc(1, len * sizeof(char *));
    strcpy(reqh->host, host);

    return 0;
}

int request_headers_add_accept_language(struct request_headers *reqh, char *acl) {
    if(!reqh) return -1;

    int len = strlen(acl);
    reqh->accept_language = calloc(1, len * sizeof(char *));
    strcpy(reqh->accept_language, acl);

    return 0;
}

int request_headers_add_user_agent(struct request_headers *reqh, char *agnt) {
    if(!reqh) return -1;

    int len = strlen(agnt);
    reqh->user_agent = calloc(1, len * sizeof(char *));
    strcpy(reqh->user_agent, agnt);

    return 0;
}

int request_headers_destroy(struct request_headers *reqh) {
    free(reqh->host);
    free(reqh->accept_language);
    free(reqh->user_agent); 

    return 0;
}


struct http_request *http_request_create() {
    struct http_request *req = calloc(1, sizeof(struct http_request));

    struct request_line *reql = request_line_create();
    struct request_headers *reqh = request_headers_create();

    req->request_line = reql;
    req->request_headers = reqh;
    req->body = NULL;

    return req;
}

int http_request_add_body(struct http_request *req, char *body) {
    if(!req) return -1;
    if(!req->request_headers->content_length) return -1;
    if(!body) return -1;

    req->body = realloc(req->body, req->request_headers->content_length);
    memset(req->body, 0, req->request_headers->content_length * sizeof(char));

    strncpy(req->body, body, req->request_headers->content_length);

    return 0;
}

int http_request_destroy(struct http_request *req) {
    request_line_destroy(req->request_line);
    request_headers_destroy(req->request_headers);
    free(req->body);
    free(req);

    return 0;
}

struct http_request *construct_http_request(char *verb, char *uri, char *host, char *lang, char *agnt, int len, char *body) {
    struct http_request *req = calloc(1, sizeof(struct http_request));

    request_line_add_http_verb(req->request_line, verb);
    request_line_add_uri(req->request_line, uri);
    
    request_headers_add_host(req->request_headers, host);
    request_headers_add_accept_language(req->request_headers, lang);
    request_headers_add_user_agent(req->request_headers, agnt);
    req->request_headers->content_length = len;

    http_request_add_body(req, body);

    return req;
}

struct http_request *parse_http_request(char *req) {

    return NULL;
}

/* ========================= HTTP RESPONSE ========================= */

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

    time_t curr_time = time(NULL);
    char *timestr = ctime(&curr_time); 

    hdr->date = calloc(1, (strlen(timestr) + 1) * sizeof(char));
    strcpy(hdr->date, timestr);

    return 0;
}

// Add the given connection, conn, to the general_headers struct, hdr
int general_headers_add_connection(struct general_headers *hdr, char *conn) {
    if(!hdr) return -1;

    hdr->connection = calloc(1, (strlen(conn) + 1) * sizeof(char));
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

    hdr->server = calloc(1, (strlen(serv) + 1) * sizeof(char));
    strcpy(hdr->server, serv);

    return 0;
}

// Add the given accept_ranges, ars, to the response_headers struct, hdr
int response_headers_add_accept_ranges(struct response_headers *hdr, char *ars) {
    if(!hdr) return -1;

    hdr->accept_ranges = calloc(1, (strlen(ars) + 1) * sizeof(char));
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

    hdr->content_type = calloc(1, (strlen(type) + 1) * sizeof(char));
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
    if(!res->entity_headers) {
        fprintf(stderr, "Error creating http_response: http_response has no entity_headers\n");
        return -1;
    }
    if(!res->entity_headers->content_length) {
        fprintf(stderr, "Error creating http_response: entity_headers has no content_length\n");
        return -1;
    }
    
    // Reallocate enough space for the whole body and zero the new memory
    res->body = realloc(res->body, res->entity_headers->content_length);
    memset(res->body, 0, res->entity_headers->content_length * sizeof(char));

    // Copy the provided body into the response
    strncpy(res->body, body, res->entity_headers->content_length);

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
    int n = dprintf(*clisockfd, "HTTP/1.1 %d %s\n"
                                "Date: %s"
                                "Connection: %s\n" 
                                "Server: %s\n"
                                "Accept-Ranges: %s\n"
                                "Content-Type: %s\n"
                                "Content-Length: %d\n\n"
                                "%s\n",
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

    if(general_headers_add_connection(res->general_headers, conn) < 0) {
        fprintf(stderr, "Error adding connection to general_headers struct\n");
        return NULL;
    }

    if(response_headers_add_server(res->response_headers, serv) < 0) {
        fprintf(stderr, "Error adding server to response_headers struct\n");
        return NULL;
    }

    if(response_headers_add_accept_ranges(res->response_headers, ars) < 0) {
        fprintf(stderr, "Error adding accept_ranges to response_headers struct\n");
        return NULL;
    }

    res->entity_headers->content_length = len;

    if(entity_headers_add_content_type(res->entity_headers, type) < 0) {
        fprintf(stderr, "Error adding content_type to entity_headers struct\n");
        return NULL; 
    } 

    if(add_http_response_body(res, body) < 0) {
        fprintf(stderr, "Error adding body to http_response\n");
        return NULL;
    }

    // Timestamp the response
    general_headers_add_timestamp(res->general_headers);

    return res;
}

