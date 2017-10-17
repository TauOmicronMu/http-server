// HTTP_REQUEST
// TODO: add structs and definitions for handling http_requests


// HTTP_RESPONSE
struct general_headers {
    char *date;
    char *connection;
};

struct response_headers {
    char *server;
    char *accept_ranges;
};

struct entity_headers {
    char *content_type;
    int content_length;
};

struct http_response {
    int status;  
    struct general_headers  *general_headers;
    struct response_headers *response_headers;
    struct entity_headers   *entity_headers;
    char *body;
};

struct general_headers *create_general_headers();
int destroy_general_headers(struct general_headers *hdr);

struct response_headers *create_response_headers();
int destroy_response_headers(struct response_headers *hdr);

struct entity_headers *create_entity_headers();
int destroy_entity_headers(struct entity_headers *hdr);

struct http_response *create_http_response();
int destroy_http_response(struct http_response *res);
int add_http_response_body(struct http_response *res, char *body);
struct http_response *parse_http_response(char *res);
int send_http_response(int *clisockfd, struct http_response *res);

