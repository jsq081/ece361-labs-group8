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
#include <time.h>
#include "packet.h"
#include "packet.c"
#include <stdbool.h>

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
    char buf[BUF_SIZE];    
    struct sockaddr_in client_addr; 
    socklen_t client_len = sizeof(client_addr); // length of client info
    // recvfrom the client and store info in cli_addr so as to send back later

    if (recvfrom(socketfd, (char*) buf, sizeof(buf), 0, (struct sockaddr *) &client_addr, &client_len) == -1) {
        printf("Error: Unseccessful recieve\n");
        exit(EXIT_FAILURE);
    }


    printf("Finished recieve, start reply\n");

    //send back msg to client
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

   Packet packet;
   packet.filename=(char*)malloc(BUF_SIZE);
   char filename[BUF_SIZE]={0};

   FILE *fptr = NULL;
   bool *fragRecv = NULL;

   while (1)
   {
      if (recvfrom(socketfd, (char*) buf, sizeof(buf), 0, (struct sockaddr *) &client_addr, &client_len) == -1) {
        printf("Error: Unseccessful recieve\n");
        exit(EXIT_FAILURE);
      }

      stringToPacket(buf, &packet);

      if(fptr==NULL){
          strcpy(filename,packet.filename);
          fptr=fopen(filename, "w"); //Creates an empty file for writing.
          fragRecv = (bool *)malloc(packet.total_frag * sizeof(fragRecv));

          for(int i=0;i<packet.total_frag;i++){
              fragRecv[i]=false;
          }
      }

      if(fragRecv[packet.frag_no]==false){
          if(fwrite(packet.filedata,sizeof(char),packet.size,fptr)!=packet.size){
              printf("cannot copy data into new file\n");
              exit(EXIT_FAILURE);
          }else{
              fragRecv[packet.frag_no]=true;
          }
      }

      //Acknowlegement set up
      strcpy(packet.filedata,"ACK");

      packetToString(buf,&packet);

      if (sendto(socketfd, buf, sizeof(buf), 0, (struct sockaddr *) &client_addr,client_len) == -1) {
            printf("Need ACK\n");
            exit(EXIT_FAILURE);
      }

      //loop to last frag, break loop
      if (packet.frag_no==packet.total_frag){

          printf("finish file transfer\n");
          break;
      }


   }
    //printf("Finished reply\n");
    close(socketfd);

    return 0;
}