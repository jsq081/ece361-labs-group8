#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
    if (argc == 1 || argc > 2) {
        printf("Execution command should be:\n server <UDP listen port>\n");
        return 0;
    }

    //1.
    int port = atoi(argv[1]);
    // create the socket
    int socket_FD = socket(AF_INET, SOCK_DGRAM, 0); //return a file descriptor for this new socket
    printf("socket_FD is: %d\n", socket_FD);

    // server set its socket address in a custom fashion
    // any available IP address is okay, but need to specify port number
    struct sockaddr_in server_addr;
    server_addr.sin_family= AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to all local interfaces
    server_addr.sin_port = htons(port);
    // Note that sin_zero (which is included to pad the structure to the length of a struct sockaddr)
    // should be set to all zeros with the function memset(). -- Beej
    // void * memset ( void * ptr, int value, size_t num );
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    struct sockaddr* saddr = &server_addr;
    socklen_t saddrlen = sizeof(server_addr);

    // associate the socket with an address
    if(bind(socket_FD, saddr, saddrlen) == -1){
        printf("Bind failed\n");
    }
    printf("finish bind, start receive\n");
    //2. receive
    char buffer[50] = {0}; // buffer is used to copy received messages into
    struct sockaddr_in client_addr;
    socklen_t caddrlen = sizeof(client_addr);
    struct sockaddr* caddr = &client_addr;

    recvfrom(socket_FD, buffer, sizeof(buffer), 0, caddr, &caddrlen); // empty client addr storage
    printf("finish receive, start send\n");
    // 3. send
    if (strcmp(buffer, "ftp") == 1){
        sendto(socket_FD, "yes", sizeof("yes"), 0, caddr, caddrlen);
    }else{
        sendto(socket_FD, "no", sizeof("no"), 0, caddr, caddrlen);
    }

    return 0;
}
