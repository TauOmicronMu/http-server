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
int general_headers_add_timestamp(struct general_headers *hdr);
int general_headers_add_connection(struct general_headers *hdr, char *conn);
int destroy_general_headers(struct general_headers *hdr);

struct response_headers *create_response_headers();
int response_headers_add_server(struct response_headers *hdr, char *serv);
int response_headers_add_accept_ranges(struct response_headers *hdr, char *ars);
int destroy_response_headers(struct response_headers *hdr);

struct entity_headers *create_entity_headers();
int entity_headers_add_content_type(struct entity_headers *hdr, char *type);
int destroy_entity_headers(struct entity_headers *hdr);

struct http_response *create_http_response(); // Create a blank HTTP response
int destroy_http_response(struct http_response *res); // Destroy an HTTP response
int add_http_response_body(struct http_response *res, char *body); // Add a body to an HTTP response
struct http_response *parse_http_response(char *res); // Parse an HTTP response into the corresponding struct
int send_http_response(int *clisockfd, struct http_response *res); // Send an HTTP response over a socket

// Construct a full HTTP response (struct) from the required components
struct http_response *construct_http_response(int status, char *conn, char *serv, char *ars, char *type, int len, char *body);
