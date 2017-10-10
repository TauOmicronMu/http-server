#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 10

#define BUFFER_SIZE 256

int main(int argc, char ** argv) {

    int portno, sockfd;
    struct sockaddr_in6 serv_addr;
    struct sockaddr_in6 cli_addr;

    // Check args
    if(argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Verify port number
    portno = atoi(argv[0]);
    if(portno < 0 || portno > 65535) {
        fprintf(stderr, "invalid port number: %d\n", portno);
        exit(1);
    }

    // Create socket 
    sockfd = socket(PF_INET6, SOCK_STREAM, 0); 
    if(sockfd < 0) {
        fprintf(stderr, "error (%d) creating socket\n", sockfd);
        exit(2);
    }

    memset((char *) &serv_addr, '\0', sizeof(serv_addr));
    
    serv_addr.sin6_family = PF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons(portno);
    
    // Bind socket to port
    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "error binding socket to port %d\n", portno);
        exit(3); 
    }
 
    if(listen(sockfd, BACKLOG) < 0) {
        fprintf(stderr, "error listening on port %d\n", portno);
        exit(4);
    }

    // Wait for & accept incoming connections
    while(1) { 
        printf("Ayyyy lmao\n");
        int clisockfd, clilen;
        char buffer[BUFFER_SIZE];
    
        clilen = sizeof(cli_addr);
        
        clisockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
 
        if(clisockfd < 0) {
            fprintf(stderr, "error accepting connection\n");
            exit(5);
        }    

        printf("Accepted connection\n");
 
        memset(buffer, '\0', BUFFER_SIZE * sizeof(char));

        if(read(clisockfd, buffer, BUFFER_SIZE - 1) < 0) {
            fprintf(stderr, "error reading from socket\n");
            exit(6);
        }

        fprintf(stdin, "Here is the message: %s\n", buffer);
        
        if(write(clisockfd, "I got your message!", 18) < 0) {
            fprintf(stderr, "error writing to socket\n");
            exit(7);
        }
 
        close(clisockfd);        

    }

    return 0;
    
}

