#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char** argv){

    if (argc != 3) {
        printf("Error: execution command should be: deliver <server address> <server port number>\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);

    //create the socket
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (socketfd < 0){
        printf("Error: unsuccessful creation of socket");
        exit (EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    // Note that sin_zero (which is included to pad the structure to the length of a struct sockaddr)
    // should be set to all zeros with the function memset(). -- Beej
    memset (serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));
    // convert input string to IP address
    inet_pton(AF_INET,argv[1],&(serv_addr.sin_addr));

    char input[100]; 
    char filename[100]; 
    

    //get user input 
    printf("Input a message of the following form\n\t ftp <file name>\n");

    scanf("%s", &input);
    scanf("%s", &filename);


    if(strcmp(input, "ftp") != 0){
        printf("Error: the command should be:\n        ftp <file name>\n\n");
        exit(EXIT_FAILURE);
    }

    // check if the file exists
    if( access( filename, F_OK ) == -1) {
        printf("File does not exists.\n");
        exit(EXIT_FAILURE);
    } 

    socklen_t serv_addr_len = sizeof(serv_addr);

    //check if msg send succsessful
    if (sendto( socketfd, "ftp", sizeof("ftp") , 0 , (struct sockaddr *) &serv_addr, serv_addr_len) == -1) {
        printf("Failed send msg\n");
        exit(EXIT_FAILURE);
    }
    

    //recv msg 
    char buf[100];
    if (recvfrom(socketfd, (char*) buf, sizeof(buf), 0, (struct sockaddr*) &serv_addr, &serv_addr_len) == -1) {
        printf("Error: Unseccessful recieve\n");
        exit(EXIT_FAILURE);
    }

    if(strcmp(buf, "yes")==0){
        printf("A file transfer can start\n");
    }else{
        exit(EXIT_FAILURE);
    }

    return 0;
}
