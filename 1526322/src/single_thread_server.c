#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 10

#define BUFFER_SIZE 16384

int main(int argc, char ** argv) {

    int portno, sockfd;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

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
        exit(2);
    }

    memset((char *) &serv_addr, '\0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    // Bind socket to port
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "error binding socket to port %d\n", portno);
        exit(3); 
    }
 
    if(listen(sockfd, BACKLOG) < 0) {
        fprintf(stderr, "error listening on port %d\n", portno);
        exit(4);
    }

    // Wait for & accept incoming connections
    while(1) { 
        int clisockfd, clilen;
        char buffer[BUFFER_SIZE];
    
        clilen = sizeof(cli_addr);
        
        if((clisockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) < 0) {
            fprintf(stderr, "error accepting connection\n");
            exit(5);
        }    
 
        memset(buffer, '\0', BUFFER_SIZE * sizeof(char));

        if(read(clisockfd, buffer, BUFFER_SIZE - 1) < 0) {
            fprintf(stderr, "error reading from socket\n");
            exit(6);
        }

        printf("Here is the message: %s", buffer);
        
        if(write(clisockfd, "I got your message!\n", 18) < 0) {
            fprintf(stderr, "error writing to socket\n");
            exit(7);
        }
 
        close(clisockfd);        

    }

    return 0;
    
}

