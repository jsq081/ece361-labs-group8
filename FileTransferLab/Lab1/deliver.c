#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int main(int argc, char** argv){

    //check user argument
    if(argc != 2){
        printf("Error: wrong agument format.\n");
        exit (EXIT_FAILURE);
    }

    //convert string to int
    int port = atoi(argv[1]); 

    //create the socket
    int socketfd = socket(AF_INET,SOCK_DGRAM,0);

    if (socketfd < 0){
        printf("Error: unsuccessful creation of socket");
        exit (EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    memset (serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));


    socklen_t serv_addr_len = sizeof(serv_addr);
    // bind socket to address
    if (bind( socketfd, (const struct sockaddr * ) &serv_addr, serv_addr_len) == -1) {
        printf("Error: bind error\n");
        exit(EXIT_FAILURE);
    }

    printf("Server reciving on port %d\n", port);


    //recieve msg
    char buf[100];  
    struct sockaddr_in client_addr; 
    socklen_t client_len = sizeof(client_addr); // length of client info
    // recvfrom the client and store info in cli_addr so as to send back later
    //printf("hahaha\n");

    if (recvfrom(socketfd, (char*) buf, sizeof(buf), 0, (struct sockaddr *) &client_addr, &client_len) == -1) {
        printf("Error: Unseccessful recieve\n");
        exit(EXIT_FAILURE);
    }


    printf("Finished recieve, start reply\n");
    //reply msg
    if (strcmp(buf, "ftp") == 0) {
        if (sendto(socketfd, "yes", sizeof("yes"), 0, (struct sockaddr *) &client_addr,client_len) == -1) {
            printf("Error: Unseccessful sendto\n");
            exit(EXIT_FAILURE);
        }
    } else {
        if (sendto(socketfd, "no", sizeof("no"), 0, (struct sockaddr *) &client_addr,client_len) == -1) {
            printf("Error: Unseccessful sendto\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("Finished reply\n");
    close(socketfd);

    return 0;
}
