#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "http.h"

#define BACKLOG 10

#define BUFFER_SIZE 16384

#define FILES "files.txt"

struct cli_thread_args {
    struct sockaddr_in *cli_addr;
    int *clisockfd;
} cli_thread_args;

char *ext2mime(char *ext) {
    if(strcmp(ext, "gif") == 0) return "image/gif";
    return "text/plain"; // TODO: check what the correct default value should be
}

/* 
 * Reads the file at the filepath, filepath, into
 * a buffer and returns a pointer to the buffer.
 */
char *readFile(char *filepath) {
    FILE *fp = fopen(filepath, "r");
    struct stat *fileStat = calloc(1, sizeof(struct stat));
    if(stat(filepath, fileStat) < 0) {
        fprintf(stderr, "Error statting %s\n", filepath);
        free(fileStat);
        return NULL;
    }
    // Allocate a buffer of the correct size
    int size = fileStat->st_size;
    printf("File is %d large\n", size);
    char *buf = calloc(size + 1, sizeof(char));
   
    // Read the file into the buffer
    fread(buf, sizeof(char), size, fp);
    printf("Here's that file! %s\n", buf);

    free(fileStat);
    return buf;
}

/* 
 * Returns whether or not a given file(path), file, should
 * be served by the server. 
 * @return  0 if the file should be served, 
 *         >0 if the file shouldn't be served,
 *         <0 if an error occured  
 */
int servable(char *file) {
    char *buf = readFile(FILES); 

    // Check each line to see if it matches
    char *rest = buf;
    char *token = NULL;
    while(token = strtok_r(rest, "\n", &rest)) {
        if(strcmp(token, file) == 0) {
            free(buf);
            return 0;
        }
    }
    free(buf);
    return 1;
}

int handleRequest(char *request, int *clisockfd) {
    // Parse the http request
    struct http_request *req = parse_http_request(request);

    struct http_response *res;
    int status = 0;
    char *conn = "close";
    char *serv = "TServer/1.0";
    char *ars = "bytes";
    char *type = "";
    char *body = "";
    int len = 0;

    char *verb = req->request_line->http_verb;
    char *uri = req->request_line->request_uri;

    if(strcmp(verb, "GET") == 0) {    
        // Check if the requested resource exists, if it
        // does, we'll send it back
        if(servable(uri) == 0) {
            status = 200;
            
            // If the uri is '/', return the index file
            if(strcmp(uri, "/") == 0) {
                type = "text/html"; 
                body = readFile("index.html");
                len = strlen(body);
            }
            else {
                // Otherwise serve the requested file  
                // First, get the file extension
                char *rest = uri;
                char *filename = strtok_r(rest, ".", &rest); 
                char *extension = strtok_r(rest, ".", &rest);
                printf("filename: %s, extension: %s\n", filename, extension);

                // Get the MIME type, based on the extension (and then set Content-Type)
                char *mime = ext2mime(extension);
                type = mime;
                printf("MIME type: %s\n", mime);
 
                // Strip the leading / from the filename and read it into the body of the response
                char *filename_c = filename + 1;
                int buf_n = strlen(filename_c) + strlen(extension) + 2; // The overall length of the uri               
                char *buf = calloc(buf_n, sizeof(char));
                snprintf(buf, buf_n, "%s.%s", filename_c, extension);
                printf("Attempting to read: %s\n", buf);
                body = readFile(buf);
                printf("read: %s\n", body);
                len = strlen(body);
                free(buf);
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

    // Construct a HTTP Response from the parameters
    res = construct_http_response(status, conn, serv, ars, type, len, body);

    if(!res) {
        fprintf(stderr, "Error constructing http_response\n");
        exit(1);
    }

    if(send_http_response(clisockfd, res) < 0) {
        fprintf(stderr, "Error sending HTTP Response\n");
        exit(1);
    }

    if(destroy_http_response(res) < 0) {
        fprintf(stderr, "Error destroying HTTP response struct\n");
        exit(1);
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
    free(cli_addr);
    free((struct cli_thread_args *) args);

    return (void *) 0;
}

int main(int argc, char ** argv) {

    int portno, sockfd;
    struct sockaddr_in serv_addr;

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
        int clilen;

        clilen = sizeof(cli_addr);
        
        if((*clisockfd = accept(sockfd, (struct sockaddr *) cli_addr, &clilen)) < 0) {
            fprintf(stderr, "error accepting connection\n");
            exit(1);
        }    

        struct cli_thread_args *args = calloc(1, sizeof(struct cli_thread_args));
        args->cli_addr = cli_addr;
        args->clisockfd = clisockfd;

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

