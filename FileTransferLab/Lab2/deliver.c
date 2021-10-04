#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <arpa/inet.h>
#include <sys/socket.h>     
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include "packet.h"

int main(int argc, char** argv){

    if (argc != 3) {
        printf("Error: wrong agument format.\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);

    //create the socket
    int socketfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    if (socketfd < 0){
        printf("Error: unsuccessful creation of socket");
        exit (EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memset (serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));

    //converts an Internet address in its standard text format 
    //into its numeric binary form
    inet_pton(AF_INET,argv[1],&(serv_addr.sin_addr));

    char input[100]; 
    //char filename[100]; 
    char filename[100] = {'\0'};
    //get user input 
    printf("Input a message of the following form\n\t ftp <file name>\n");

    scanf("%s %s", input,filename);


    /*no input for filename case
    if (filename[0] == '\0') {
      printf("Error: Need enter filename\n");
      return 1;
    }*/


    if(strcmp(input, "ftp") != 0){
        printf("Error: Need to type ftp.\n");
        exit(EXIT_FAILURE);
    }

    // check if the file exists
    if( access( filename, F_OK ) == -1) {
        printf("File does not exists.\n");
        exit(EXIT_FAILURE);
    } 

    socklen_t serv_addr_len = sizeof(serv_addr);

    //send msg
    clock_t start,end; //set up time vars

    start = clock();

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

    end = clock();
    printf("RTT=%f us.\n",((double)(end-start)*1000000/CLOCKS_PER_SEC));

    if(strcmp(buf, "yes")==0){
        printf("A file transfer can start\n");
    }else{
        exit(EXIT_FAILURE);
    }

    //Sec 2.2
    // client open file
    FILE* file = fopen(filename, "r");  // open for reading
    if (file == NULL){
        printf("Error: failed open file\n");
        exit(EXIT_FAILURE);
    }
    // read data from file and contruct packet
    // send packet info in a string array to server
    // seek to the end of file and check the position
    fseek(file, 0, SEEK_END); // seek to end of file
    // each packet contains one frag
    // num_frag starts from 1
    long int num_frag = ftell(file) / 1000 + 1;   // current value of the position indicator of the stream
    fseek(file, 0, SEEK_SET); // seek back to the beginning
    //printf("number of fragments/packets: %ld\n", num_frag);

    // each packet is a char array
    // packets should be double pointer
    struct Packet packets[num_frag];
    char* pac_form[num_frag];

   for (int i = 0; i < num_frag; ++i){
        packets[i].total_frag = num_frag;
        // num_frag starts from 1
        packets[i].frag_no = i+1;
        //printf("total flag: %d, flag no: %d\n", packets[i].total_frag, packets[i].frag_no);
        packets[i].size = fread(packets[i].filedata, 1, 1000, file);
        //printf("size:%d\n", packets[i].size);
        packets[i].filename = filename;
        //transfer packet into char array format
        //initialize char array
        //printf("initialize char\n");
        //memset(pac_form[i], 0, 1200);
        //int str_size = sprintf(NULL, )
        pac_form[i] = malloc(1200);
        // int curr = 0;
        // convert in to string
        // return the total number of chars written
        //printf("aaa\n");
        int cursor = sprintf(pac_form[i], "%d:%d:%d:%s:", packets[i].total_frag,
                                                packets[i].frag_no,
                                                packets[i].size,
                                                packets[i].filename);
        //printf("ready to copy\n");
        memcpy(pac_form[i]+cursor, packets[i].filedata, packets[i].size);
        //printf("1.packet ---%s---\n", pac_form[i]);
    }
    
    // client send packets
    for (int i = 0; i < num_frag; ++i){
        //sendto(socketfd, pac_form[i], sizeof(pac_form[i]), 0,  (struct sockaddr *) &serv_addr, serv_addr_len);
        if (sendto(socketfd, pac_form[i], 1200, 0,  (struct sockaddr *) &serv_addr, serv_addr_len) == -1){
            printf("Failed send packets\n");
            exit(EXIT_FAILURE);
        }
        //printf("2.packet ---%s---\n", pac_form[i]);
    }

    close(socketfd);

    
    return 0;
}
