#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "http.h"

#define BACKLOG 10

#define BUFFER_SIZE 16384

struct cli_thread_args {
    struct sockaddr_in *cli_addr;
    int *clisockfd;
} cli_thread_args;

int handleRequest(char *request, int *clisockfd) {
        
    // TODO: Actually handle requests instead of sending a default OK one.
   
    // construct OK response with default page. 
    int status = 200;
    char *conn = "close";
    char *serv = "TServer/1.0";
    char *ars = "bytes";
    char *type = "text/html";
    char *body = "<!doctype html><html><body><h1>TEST</h1></body></html>\n";  
    int len = strlen(body);

    struct http_response *res = construct_http_response(status, conn, serv, ars, type, len, body);
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

