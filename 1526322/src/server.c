#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "http.h"
#include "freer.h"

#define BACKLOG 10

#define BUFFER_SIZE 16384

#define FILES "files.txt"

// Used to make sure everything's cleaned up at the end
struct free_list *freelist;

struct cli_thread_args {
    struct sockaddr_in *cli_addr;
    int *clisockfd;
} cli_thread_args;

char *ext2mime(char *ext) {
    // Images
    if(strcmp(ext, "jpg") == 0) return "image/jpeg";
    if(strcmp(ext, "png") == 0) return "image/png";
    if(strcmp(ext, "gif") == 0) return "image/gif";
    if(strcmp(ext, "bmp") == 0) return "image/bmp";
    // Text
    if(strcmp(ext, "txt") == 0) return "text/plain";
    if(strcmp(ext, "html") == 0) return "text/html";
    if(strcmp(ext, "css") == 0) return "text/css";
    if(strcmp(ext, "js") == 0) return "text/javascript";
    // Binary data
    if(strcmp(ext, "pdf") == 0) return "application/pdf";
    if(strcmp(ext, "xml") == 0) return "application/xml";
    // Default
    return "text/plain";
}

// Print a string, str, of size n (regardless of null bytes)
void printnull(char *str, int n) {
    int i;
    for(i = 0; i < n; i++) printf("%c", str[i]);
}

// Struct that represents a file (specifically one read by readFile)
struct read_file {
    int len;
    char *body;
};

/* 
 * Reads the file at the filepath, filepath, into
 * a buffer and returns a pointer to the buffer.
 */
struct read_file *readFile(char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    struct stat *fileStat = calloc(1, sizeof(struct stat));
    free_list_add(fileStat, freelist);
    if(stat(filepath, fileStat) < 0) {
        fprintf(stderr, "Error statting %s\n", filepath);
        free(fileStat);
        fileStat = NULL;
        return NULL;
    }
    // Allocate a buffer of the correct size
    int size = fileStat->st_size;

    struct read_file *file = calloc(1, sizeof(struct read_file));
    file->body = calloc(size + 1, sizeof(char));
    file->len = size;

    free_list_add(file, freelist);
    free_list_add(file->body, freelist);
   
    // Read the file into the buffer
    fread(file->body, size, 1,  fp);

    fclose(fp); 

    free(fileStat);
    fileStat = NULL;
    return file;
}

/* 
 * Returns whether or not a given file(path), file, should
 * be served by the server. 
 * @return  0 if the file should be served, 
 *         >0 if the file shouldn't be served,
 *         <0 if an error occured  
 */
int servable(char *file) {
    char *buf = readFile(FILES)->body; 

    // Check each line to see if it matches
    char *rest = buf;
    char *token = NULL;
    while( (token = strtok_r(rest, "\n", &rest)) ) {
        if(strcmp(token, file) == 0) {
            free(buf);
            buf = NULL;
            return 0;
        }
    }
    free(buf);
    buf = NULL;
    return 1;
}

int handleRequest(char *request, int *clisockfd) {
    // Parse the http request
    struct http_request *req = parse_http_request(request);
    free_list_add(req->request_line->http_verb, freelist);
    free_list_add(req->request_line->request_uri, freelist);
    free_list_add(req->request_line, freelist);
    free_list_add(req->request_headers->host, freelist);
    free_list_add(req->request_headers->accept_language, freelist);
    free_list_add(req->request_headers->user_agent, freelist);
    free_list_add(req->request_headers, freelist);
    free_list_add(req->body, freelist);
    free_list_add(req, freelist);

    struct read_file *file = NULL;
    struct http_response *res;
    int status = 0;
    char *conn = "close";
    char *serv = "TServer/1.0";
    char *ars = "bytes";
    char *type = "text/plain";
    char *body = " ";
    int len = 1;

    char *verb = req->request_line->http_verb;
    char *uri = req->request_line->request_uri;
    if(!verb) verb = "";
    if(!uri) uri = "";

    if(strcmp(verb, "GET") == 0) {    
        // Check if the requested resource exists, if it
        // does, we'll send it back
        if(servable(uri) == 0) {
            status = 200;
            
            // If the uri is '/', return the index file
            if(strcmp(uri, "/") == 0) {
                type = "text/html"; 
                file = readFile("index.html");
                body = file->body;
                len = file->len;
            }
            else {
                // Otherwise serve the requested file  
                // First, get the file extension
                char *rest = uri;
                char *filename = strtok_r(rest, ".", &rest); 
                char *extension = strtok_r(rest, ".", &rest);

                // Get the MIME type, based on the extension (and then set Content-Type)
                char *mime = ext2mime(extension);
                type = mime;
 
                // Strip the leading / from the filename and read it into the body of the response
                char *filename_c = filename + 1;
                int buf_n = strlen(filename_c) + strlen(extension) + 2; // The overall length of the uri               
                char *buf = calloc(buf_n, sizeof(char));
                free_list_add(buf, freelist);
                snprintf(buf, buf_n, "%s.%s", filename_c, extension);
                file = readFile(buf);
                body = file->body;
                len = file->len;
            }
        }
        else {
            // The resource doesn't exist, so return a 404 (Not Found)
            status = 404;
        }
    }
    else {
        // We didn't recognise this, so send back a 400 (Bad Request)
        status = 400;
    }

    // Construct an HTTP Response from the parameters
    res = construct_http_response(status, conn, serv, ars, type, len, body);

    free_list_add(res->general_headers->date, freelist);
    free_list_add(res->general_headers->connection, freelist);
    free_list_add(res->general_headers, freelist);
    free_list_add(res->response_headers->server, freelist);
    free_list_add(res->response_headers->accept_ranges, freelist);
    free_list_add(res->response_headers, freelist);
    free_list_add(res->entity_headers->content_type, freelist);
    free_list_add(res->entity_headers, freelist);
    free_list_add(res->body, freelist);
    free_list_add(res, freelist);

     if(!res) {
        fprintf(stderr, "Error constructing http_response\n");
        exit(1);
    }

    if(send_http_response(clisockfd, res) < 0) {
        fprintf(stderr, "Error sending HTTP Response\n");
        exit(1);
    }

    // Clean up
    if(http_request_destroy(req) < 0) {
        fprintf(stderr, "Error destroying HTTP request struct\n");
    }

    if(destroy_http_response(res) < 0) {
        fprintf(stderr, "Error destroying HTTP response struct\n");
    }

    return 0;
}

void *handleConnection(void *args) {
    int *clisockfd = ((struct cli_thread_args *) args)->clisockfd;
    struct sockaddr_in *cli_addr = ((struct cli_thread_args *) args)->cli_addr;

    char in_buffer[BUFFER_SIZE];

    memset(in_buffer, '\0', BUFFER_SIZE * sizeof(char));

    if(read(*clisockfd, in_buffer, BUFFER_SIZE - 1) < 0) {
        fprintf(stderr, "error reading from socket\n");
        exit(1);
    }

    if(handleRequest(in_buffer, clisockfd) < 0) {
        fprintf(stderr, "error handling request:\n%s\n", in_buffer);
        exit(1);
    }

    close(*clisockfd);
    free(clisockfd);
    clisockfd = NULL;
    free(cli_addr);
    cli_addr = NULL;
    free((struct cli_thread_args *) args);
    args = NULL; 

    return (void *) 0;
}

void sigintHandler() { 
    free_list_free_all(freelist);
    exit(0); 
}

int main(int argc, char ** argv) {
    int portno, sockfd;
    struct sockaddr_in serv_addr;

    freelist = free_list_create();
    signal(SIGINT, *sigintHandler);

    // Check args
    if(argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Verify port number
    portno = strtoul(argv[1], NULL, 10);
    if(portno < 0 || portno > 65535) {
        fprintf(stderr, "invalid port number: %d\n", portno);
        exit(1);
    }

    // Create socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(sockfd < 0) {
        fprintf(stderr, "error (%d) creating socket\n", sockfd);
        exit(1);
    }

    // Allow socket to be reused (it's a PITA otherwise...)
    if( (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) ) {
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed");
    }

    memset((char *) &serv_addr, '\0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    // Bind socket to port
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "error binding socket to port %d\n", portno);
        exit(1); 
    }
 
    if(listen(sockfd, BACKLOG) < 0) {
        fprintf(stderr, "error listening on port %d\n", portno);
        exit(1);
    }

    // Wait for & accept incoming connections
    while(1) { 
        struct sockaddr_in *cli_addr = calloc(1, sizeof(struct sockaddr));
        int *clisockfd = calloc(1, sizeof(int));
        unsigned int clilen;

        free_list_add(cli_addr, freelist);
        free_list_add(clisockfd, freelist);

        clilen = sizeof(cli_addr);
        
        if((*clisockfd = accept(sockfd, (struct sockaddr *) cli_addr, &clilen)) < 0) {
            fprintf(stderr, "error accepting connection\n");
            exit(1);
        }    

        struct cli_thread_args *args = calloc(1, sizeof(struct cli_thread_args));
        args->cli_addr = cli_addr;
        args->clisockfd = clisockfd;

        free_list_add(args, freelist);

        // Create a new thread
        pthread_t cli_thread;
        if(pthread_create(&cli_thread, NULL, &handleConnection, args) < 0) {
            fprintf(stderr, "error creating thread\n");
            exit(1);
        }

        pthread_join(cli_thread, NULL);
    }

    return 0;
    
}

