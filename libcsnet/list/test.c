#include "lflist.h"

#include <stdio.h>
#include <stdlib.h>

int main()
{
	struct lflist* l = lflist_new();
	struct hp_record* record = allocate_thread_private_hp_record(l->hp);
	lflist_add(l, record, 1);
	lflist_del(l, record, 1);
	lflist_del(l, record, 1);
	lflist_free(l);

	return 0;
}

