#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
	struct timer* timer = timer_init(30, 709);

	for (int i = 0xab; i < 0xab + 70; i++) {
		usleep(500000);
		int which_slot = timer_book_keeping(timer);
		if (which_slot > 0) {
			/* TODO: We got a hashtable here, how to let event_loop close the connection
			   and remove them from timer */
			int_hash_t* hashtable = timer->timer_hash[which_slot];
			for (int j = 0; j < hashtable->slot_count; j++) {
				int_hash_node_t* node = hashtable->lists[j];
				while (node) {
					int_hash_node_t* tmp = node;
					node = node->next;
					struct timer_node* tnode = tmp->value;
					printf("%d %d\n", tnode->timerid, tnode->interval);
					timer_remove(timer, tnode->timerid);
				}
			}
		}

		timer_insert(timer, i, i);
	}

	printf("total timer: %d\n", timer->timerid_hash->key_count);

	sleep(3);

	int which_slot = timer_book_keeping(timer);
	if (which_slot > 0) {
		int_hash_t* hashtable = timer->timer_hash[which_slot];
		for (int j = 0; j < hashtable->slot_count; j++) {
			int_hash_node_t* node = hashtable->lists[j];
			while (node) {
				int_hash_node_t* tmp = node;
				node = node->next;
				struct timer_node* tnode = tmp->value;
				printf("%d %d\n", tnode->timerid, tnode->interval);
				timer_remove(timer, tnode->timerid);
			}
		}
	}

	printf("total timer: %d\n", timer->timerid_hash->key_count);

	timer_terminate(timer);

	return 0;
}
