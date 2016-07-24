#pragma once

#include "csnet-log.h"
#include "csnet-config.h"

typedef struct csnet_register {
	int mytype;
	char* myip;
	int myport;
	int lookup_server_fd;
	char* accept_server_type;
	int* accept_server_type_list;
	int accept_server_type_count;
	char* connect_server_type;
	int* connect_server_type_list;
	int connect_server_type_count;
	csnet_log_t* log;
} csnet_register_t;

csnet_register_t* csnet_register_new(csnet_config_t* config, csnet_log_t* log);
void csnet_register_free(csnet_register_t* reg);

int csnet_register_myself(csnet_register_t* reg);
int csnet_register_notice(csnet_register_t* reg);

