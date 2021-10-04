#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 

struct packet {
    unsigned int total_frag; 
    unsigned int frag_no; 
    unsigned int size; 
    char* filename; 
    char filedata[1000];
};
