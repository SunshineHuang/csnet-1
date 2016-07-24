#pragma once

#include "cs-hashtable.h"

typedef struct csnet_config {
	cs_ht_t* hashtbl;
} csnet_config_t;

csnet_config_t* csnet_config_new();
void csnet_config_free(csnet_config_t*);
void csnet_config_load(csnet_config_t*, const char* file);
void* csnet_config_find(csnet_config_t*, void* key, int key_len);

