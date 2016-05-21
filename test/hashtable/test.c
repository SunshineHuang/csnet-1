#include "hashtable.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* keys[] = {
	"server_no",
	"server_type",
	"server_name",
	"listen_port",
	"maxconn",
	"out_time",
	"client_max_conn",
	"model",
	"log",
	"log_level",
	"log_size",
	"msg_stat_path",
	"msg_stat_size",
	"msg_stat_key",
	"minganci_path",
	"max_pk_num",
	"pk_out_time",
	"save_msg_size",
	"flow_A",
	"ip_A",
	"port_A",
	"type_A",
	"flow_B",
	"ip_B",
	"port_B",
	"type_B",
	"conn_num",
	"flow_1",
	"ip_1",
	"port_1",
	"type_1",
	"flow_2",
	"ip_2",
	"port_2",
	"type_2",
	NULL
};

int main()
{
	struct config* config = config_create();
	config_load(config, "server.conf");

	char** p = keys;
	while (*p != NULL) {
		char* v = config_find(config, *p, strlen(*p));
		printf("%s, %s\n", *p, v);
		p++;
	}
	config_destroy(config);
	return 0;
}
