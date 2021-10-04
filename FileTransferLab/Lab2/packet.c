#include "packet.h"

void packetToString(char *buf, Packet *packet){

    int cursor = 0; 

    //memset copies the chars to the first n chars of string pointed to by str
    //num of buf_size bytes to be set to 0 (initialize string buffer)
	memset(buf, 0, BUF_SIZE); 
	
    //load the data to string
	sprintf(buf, "%d", packet -> total_frag);
    cursor = strlen(buf);
    memcpy(buf + cursor, ":", sizeof(char));
    cursor++;	
	
	sprintf(buf + cursor, "%d", packet -> frag_no);
    cursor = strlen(buf);
    memcpy(buf + cursor, ":", sizeof(char));
    cursor++;
	
	sprintf(buf  + cursor, "%d", packet -> size);
    cursor = strlen(buf);
    memcpy(buf + cursor, ":", sizeof(char));
    cursor++;
	
	sprintf(buf+cursor, "%s", packet -> filename);
    cursor =cursor + strlen(packet->filename);
    memcpy(buf+cursor, ":", sizeof(char));
    cursor++;
	
	memcpy(buf+cursor, packet->filedata, sizeof(char)*DATA_SIZE);

}

void stringToPacket(char *buf, Packet *packet){

    //compile regular expression into ":"
	regex_t reg; 
	if(regcomp(&reg, "[:]", REG_EXTENDED)) {
        fprintf(stderr, "regcomp error\n");
    }
	
	regmatch_t match[1];
	int cursor = 0; 
	char regex_buf[BUF_SIZE]; 
	
    //compare string to regular expression to find ":"
    //match total frag
    if(regexec(&reg, buf + cursor, 1, match, REG_NOTBOL)) {
        fprintf(stderr, "regex error\n");
        exit(1);
    }
    memset(regex_buf, 0, BUF_SIZE * sizeof(char));
    memcpy(regex_buf, buf + cursor, match[0].rm_so);
    packet -> total_frag = atoi(regex_buf);
    cursor =cursor + (match[0].rm_so + 1);
	

	// Get frag_no, match frag_no
	if(regexec(&reg, buf + cursor, 1, match, REG_NOTBOL)) {
        fprintf(stderr, "regex error\n");
        exit(1);
    }
	memset(regex_buf, 0, BUF_SIZE * sizeof(char));
    memcpy(regex_buf, buf + cursor, match[0].rm_so);
    packet -> frag_no = atoi(regex_buf);
    cursor = cursor + (match[0].rm_so + 1);
	
	// printf("frag no: %d\n", packet -> frag_no);

	// Get size 
	if(regexec(&reg, buf + cursor, 1, match, REG_NOTBOL)) {
        fprintf(stderr, "regex error\n");
        exit(1);
    }
	memset(regex_buf, 0, BUF_SIZE * sizeof(char));
    memcpy(regex_buf, buf + cursor, match[0].rm_so);
    packet -> size = atoi(regex_buf);
    cursor = cursor + (match[0].rm_so + 1);
		
	// Get filename
	if(regexec(&reg, buf + cursor, 1, match, REG_NOTBOL)) {
        fprintf(stderr, "regex error\n");
        exit(1);
    }
    memcpy(packet->filename, buf + cursor, match[0].rm_so);
    packet -> filename[match[0].rm_so] = 0;
    cursor += (match[0].rm_so + 1);
	
	// Get data 
	memcpy(packet->filedata, buf + cursor, packet->size);

}

//check packet output
void printPacket(Packet *packet){
    char pData[DATA_SIZE+1]={0};
    printf("total_frag=%d, frag_no=%d, size=%d, filename=%s\n", packet->total_frag,packet->frag_no,packet->size,packet->filename);
    memcpy(pData,packet->filedata,DATA_SIZE);
    printf("packet data:%s",pData);
}
