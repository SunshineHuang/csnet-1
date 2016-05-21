#include "../../src/recv_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)

struct head {                   
	unsigned int    len;    
	unsigned short  cmd;    
	unsigned short  status; 
	unsigned short  version;
	unsigned short  encry;  
	unsigned int    seq;    
	unsigned int    id;     
};

#pragma pack(pop)

int main(int argc, char** argv)
{
	if (argc != 3) {
		printf("%s arg1 arg2\n\targ1: package no. per buffer\n\targ2: total packages\n", argv[0]);
		return -1;
	}

	int n1 = atoi(argv[1]);
	int n2 = atoi(argv[2]);
	struct recv_buffer* buff = recv_buffer_create(64 * 1024);

	char data[27];
	strcpy(data, "abcdefghijklmnopqrstuvwxyz");

	struct head h;

	h.len = 20 + 26;

	char package[(20 + 26) * n1];
	int i;
	int pos = 0;

	for (i = 0; i < n1; i++) {
		memcpy(package + pos, &h, 20);
		pos += 20;
		memcpy(package + pos, data, 26);
		pos += 26;
	}

	for (i = 0; i < n2 / n1; i++) {
		recv_buffer_append(buff, package, 46 * n1);

		while (1) {
			char* p = recv_buffer_data(buff);
			struct head* ph = (struct head*)p;
			if (recv_buffer_seek(buff, ph->len) == 0) {
				break;
			}
		}
	}

	recv_buffer_destroy(buff);

	return 0;
}

