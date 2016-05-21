#include "../../src/pack.h"
#include "../../src/unpack.h"

#include <stdio.h>
#include <stdlib.h>

struct s {
	char c;
	short s;
	int i;
	long l;
};

int main(int argc, char** argv)
{
	int i;
	for (i = 0; i < atoi(argv[1]); i++) {
		struct pack pk;
		pack_init(&pk);

		struct s s;
		s.c = 'c';
		s.s = 10;
		s.i = 10;
		s.l = 10;
		char* body = "helloworldworldworldworldworldworld";

		pack_putc(&pk, s.c);
		pack_puts(&pk, s.s);
		pack_puti(&pk, s.i);
		pack_putl(&pk, s.l);
		pack_putstr(&pk, body);

		char* data = pk.data;
		int len = pk.len;

		struct unpack upk;
		unpack_init(&upk, data, len);

		char c;
		short ss;
		int i;
		long l;
		const char* b;
		c = unpack_getc(&upk);
		ss = unpack_gets(&upk);
		i = unpack_geti(&upk);
		l = unpack_getl(&upk);
		b = unpack_getstr(&upk);

		//printf("%c %d %d %ld %s\n", c, ss, i, l, b);
	}

	return 0;
}

