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
#include <math.h>
#include <sys/time.h>

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

    clock_t start, end;
    printf("Start sending from client...\n");
    start = clock();
    socklen_t serv_addr_len = sizeof(serv_addr);

    //check if msg send succsessful
    if (sendto( socketfd, "ftp", sizeof("ftp") , 0 , (struct sockaddr *) &serv_addr, serv_addr_len) == -1) {
        printf("Failed send msg\n");
        exit(EXIT_FAILURE);
    }
    

    //recv msg 
    char buf[100];
    if (recvfrom(socketfd, (char*) buf, sizeof(buf), 0, (struct sockaddr*) &serv_addr, &serv_addr_len) == -1) {
        printf("Error: Unseccessful receive\n");
        exit(EXIT_FAILURE);
    }

    end = clock();
    long int diff = end - start;
    printf("Finish receiving from server...\n");
    printf("Round-trip time: %ldus\n", diff);
    // double RTT = (end - start) / CLOCKS_PER_SEC;
    // printf("RTT: %s\n", RTT);

    if(strcmp(buf, "yes")==0){
        printf("A file transfer can start\n");
    }else{
        exit(EXIT_FAILURE);
    }

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

    struct packet packets[num_frag];
    char* pac_form[num_frag];
    for (int i = 0; i < num_frag; ++i){
        packets[i].total_frag = num_frag;
        // num_frag starts from 1
        packets[i].frag_no = i+1;
        packets[i].size = fread(packets[i].filedata, 1, 1000, file);
        packets[i].filename = filename;
        pac_form[i] = malloc(1200);
        int cursor = sprintf(pac_form[i], "%d:%d:%d:%s:", packets[i].total_frag,
                                                packets[i].frag_no,
                                                packets[i].size,
                                                packets[i].filename);
        memcpy(pac_form[i]+cursor, packets[i].filedata, packets[i].size);
    }
    
    int retransmit = 0;
    double sampleRTT = 0;
    double estimatedRTT = 0;
    double devRTT = 0;
    double timeout = 1; // initial timeout
    struct timeval begin, ending;
    struct timeval tv;

    // time: wait for timeout seconds for 
    fd_set readfds;
    FD_ZERO(&readfds);

    // client send packets
    int i = 0;
    while (i < num_frag){
        
        gettimeofday(&begin, 0);
        if (sendto(socketfd, pac_form[i], 1200, 0,  (struct sockaddr *) &serv_addr, serv_addr_len) == -1){
            printf("Failed send packets\n");
            exit(EXIT_FAILURE);
        }

        // initialize tv = timeout
        // Reference: Beej's Guide to Network Programming pp.50-51
        tv.tv_sec = timeout / 1;
        tv.tv_usec = (timeout - tv.tv_sec) * 1e6;

        FD_SET(socketfd, &readfds);

        // wait for "timeout" seconds for receiving ACK
        select(socketfd+1, &readfds, NULL, NULL, &tv);

        // check timeout, need to retransmit
        if (!FD_ISSET(socketfd, &readfds)){
            printf("Time out! Timeout = %fs, frag no = %d\n", timeout, i);
            retransmit = 1;
            continue; // skip the incrementing of frag no
        }

        //printf("2.packet ---%s---\n", pac_form[i]);
        char ack[100];
        if (recvfrom(socketfd, ack, sizeof(ack), 0, (struct sockaddr*) &serv_addr, &serv_addr_len) == -1) {
            printf("Error: Unseccessful receive ack\n");
            exit(EXIT_FAILURE);
        }

        gettimeofday(&ending, 0);

        if (strcmp(ack, "ACK") != 0){
            printf("Error: client ACK failed\n");
            exit(EXIT_FAILURE);
        }

        if (retransmit == 0){ // did not retransmit
            // measured time for transmission including send and receive
            // sampleRTT = (double) (end - start) / CLOCKS_PER_SEC;
            sampleRTT = (ending.tv_sec - begin.tv_sec) + (ending.tv_usec - begin.tv_usec) * 1e-6;
            estimatedRTT = (1 - 0.125) * estimatedRTT + 0.125 * sampleRTT;
            devRTT = (1 - 0.25) * devRTT + 0.25 * fabs(sampleRTT - estimatedRTT);
            timeout = estimatedRTT + 4 * devRTT;
        } else { 
            // reset retransmit signal if the current is needed to be retransmit
            // ignore the RTT for this segment as the ACK received may be delayed
            // which leads to inaccurate calculation of timeout
            retransmit = 0;
        }
        i++;
    }
    printf("File transfer successful\n");

    for (int i = 0; i < num_frag; ++i){
        free(pac_form[i]);
    }

    close(socketfd);
    return 0;
}
