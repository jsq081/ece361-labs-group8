#ifndef PACKET_H
#define PACKET_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#define BUF_SIZE 1200
#define DATA_SIZE 1000


typedef struct Packet {
  unsigned int total_frag;
  unsigned int frag_no;
  unsigned int size;
  char* filename;
  char filedata[DATA_SIZE];
}Packet;

void packetToString(char *buf, Packet *packet);

void stringToPacket(char *buf, Packet *packet);

void printPackt(Packet *packet);



#endif