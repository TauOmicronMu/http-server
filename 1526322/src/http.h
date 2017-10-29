// HTTP_REQUEST
struct request_line {
    char *http_verb;
    char *request_uri;
};

struct request_headers {
    char *host;
    char *accept_language;
    char *user_agent;
    int content_length;
};

struct http_request {
    struct request_line *request_line;
    struct request_headers *request_headers;
    char *body;
};

struct request_line *request_line_create();
int request_line_add_http_verb(struct request_line *reql, char *verb);
int request_line_add_uri(struct request_line *reql, char *uri);
int request_line_destroy(struct request_line *reql);

struct request_headers *request_headers_create();
int request_headers_add_host(struct request_headers *reqh, char *host);
int request_headers_add_accept_language(struct request_headers *reqh, char *acl);
int request_headers_add_user_agent(struct request_headers *reqh, char *agnt);
int request_headers_destroy(struct request_headers *reqh);

struct http_request *http_request_create();
int http_request_add_body(struct http_request *req, char *body);
int http_request_destroy(struct http_request *req);
struct http_request *construct_http_request(char *verb, char *uri, char *host, char *lang, char *agnt, int len, char *body);
struct http_request *parse_http_request(char *req);

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
int send_http_response(int *clisockfd, struct http_response *res); // Send an HTTP response over a socket

// Construct a full HTTP response (struct) from the required components
struct http_response *construct_http_response(int status, char *conn, char *serv, char *ars, char *type, int len, char *body);
