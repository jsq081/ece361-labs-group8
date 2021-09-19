#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
    if (argc != 2) {
        printf("Error: execution command should be:\n server <UDP listen port>\n");
        return 0;
    }

    //1.
    int port = atoi(argv[1]);
    // create the socket
    int socket_FD = socket(AF_INET, SOCK_DGRAM, 0); //return a file descriptor for this new socket

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to all local interfaces
    server_addr.sin_port = htons(port);
    // Note that sin_zero (which is included to pad the structure to the length of a struct sockaddr)
    // should be set to all zeros with the function memset(). -- Beej
    // void * memset ( void * ptr, int value, size_t num );
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    struct sockaddr* saddr = &server_addr;
    socklen_t saddrlen = sizeof(server_addr);

    // associate the server with specific address
    if(bind(socket_FD, saddr, saddrlen) == -1){
        printf("Error: bind failed\n");
    }
    printf("Server receiving on port %d\n", port);
    //2. receive message from client
    char buffer[100] = {'0'}; // used to copy received messages into
    struct sockaddr_in client_addr;
    socklen_t caddrlen = sizeof(client_addr);
    struct sockaddr* caddr = &client_addr;

    recvfrom(socket_FD, buffer, sizeof(buffer), 0, caddr, &caddrlen); // empty client addr storage
    printf("-----Receive finished. Start to reply...-----\n");
    // 3. reply message
    if (strcmp(buffer, "ftp") == 0){
        sendto(socket_FD, "yes", sizeof("yes"), 0, caddr, caddrlen);
    }else{
        sendto(socket_FD, "no", sizeof("no"), 0, caddr, caddrlen);
    }
    printf("-----Reply finished.-----\n");

    return 0;
}
