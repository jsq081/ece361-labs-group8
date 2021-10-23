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
#include "packet.h"

double uniform_rand(){
    return (double) rand() / (double) RAND_MAX;
}

int main(int argc, char** argv){

    //check user argument
    if(argc != 2){
        printf("Error: execution command should be:\n server <UDP listen port>\n");
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
    // Note that sin_zero (whsich is included to pad the structure to the length of a struct sockaddr)
    // should be set to all zeros with the function memset(). -- Beej
    memset (serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));


    socklen_t serv_addr_len = sizeof(serv_addr);
    // bind socket to address
    if (bind( socketfd, (const struct sockaddr * ) &serv_addr, serv_addr_len) == -1) {
        printf("Error: bind error\n");
        exit(EXIT_FAILURE);
    }

    printf("Server receiving on port %d\n", port);


    //recieve msg
    char buf[1200];  
    struct sockaddr_in client_addr; 
    socklen_t client_len = sizeof(client_addr); // length of client info
    // recvfrom the client and store info in cli_addr so as to send back later
    

    if (recvfrom(socketfd, (char*) buf, sizeof(buf), 0, (struct sockaddr *) &client_addr, &client_len) == -1) {
        printf("Error: Unseccessful receive\n");
        exit(EXIT_FAILURE);
    }

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

    FILE* file = NULL;
    struct packet pack;
    char receive_buf[1200];
    pack.filename = (char*)malloc(512*sizeof(char));
    memset(pack.filedata, 0, 1000);
    int valid_frag = 1;
    while(1){
        // receive pack from client
        if (recvfrom(socketfd, receive_buf, sizeof(receive_buf), 0, (struct sockaddr *) &client_addr, &client_len) == -1){
            printf("Error: Unseccessful packet receive\n");
            exit(EXIT_FAILURE);
        }

        if (uniform_rand() > 1e-2){
        // convert string to packet
        char* total_frag, *frag_no, *size, *filename, *filedata;
        total_frag = strtok(receive_buf, ":");
        frag_no = strtok(NULL, ":");
        size = strtok(NULL, ":");
        pack.filename = strtok(NULL, ":");

        pack.total_frag = atoi(total_frag);
        pack.frag_no = atoi(frag_no);
        pack.size = atoi(size);
        if (valid_frag == pack.frag_no){ // avoid duplicates
        valid_frag++;

        int header = strlen(total_frag) + strlen(frag_no) + strlen(size) + strlen(pack.filename) + 4;
        memcpy(pack.filedata, receive_buf+header, sizeof(char)*1000);

        if (pack.frag_no == 1){
            file = fopen(pack.filename, "w");
        }

        fwrite(pack.filedata, sizeof(char), pack.size, file);
        
        sendto(socketfd, "ACK", sizeof("ACK"), 0, (struct sockaddr *) &client_addr,client_len);
        
        if (pack.frag_no == pack.total_frag){
            break;
        }
        }
        }else {
            printf("Packet dropped\n");
        }
    }

    printf("receive file successful\n");
    close(socketfd);

    return 0;
}
