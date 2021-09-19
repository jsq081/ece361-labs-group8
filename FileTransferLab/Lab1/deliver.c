#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv){
    if (argc != 3) {
        printf("Error: execution command should be: deliver <server address> <server port number>\n");
        return 0;
    }

    //1. ask the user to input message
    printf("Input the message as follows:\n        ftp <file name>\n");
    char user_input[100] = {'0'};
    char command[100] = {'0'};
    char filename[100] = {'0'};
    //fgets(user_input, 100, stdin); // stdin: read from the standard input
    scanf("%s", &command);
    scanf("%s", &filename);

    if (strcmp(command, "ftp") != 0){
        printf("Error: the command should be:\n        ftp <file name>\n");
        return 0;
    }

    int port = atoi(argv[2]);
    // server address = argc[1]
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    // convert input string to IP address
    inet_pton(AF_INET, argv[1], &(server_addr.sin_addr));
    // Note that sin_zero (which is included to pad the structure to the length of a struct sockaddr)
    // should be set to all zeros with the function memset(). -- Beej
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    // 2. check the existence of the file
    if( access(filename, F_OK) == 0 ) {
        printf("-----File exist, sending ftp to the server...-----\n");
        if (sendto(socket_fd, "ftp", sizeof("ftp"), 0, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1){
            printf("Error: send failed\n");
            return 0;
        }
    } else {
        printf("File \"%s\" does not exist\n", filename);
        return 0;
    }

    // 3. receive message from server
    char buffer[100] = {'\0'};
    recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr*) &server_addr, sizeof(server_addr));
    printf("-----Receiving server message...-----\n");
    if (strcmp(buffer, "yes") == 0){
        printf("A file transfer can start.\n");
    }else {
        return 0;
    }
    
    return 0;
}
