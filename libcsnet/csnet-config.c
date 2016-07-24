#include "csnet-config.h"
#include "csnet-utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

csnet_config_t*
csnet_config_new() {
	csnet_config_t* conf = calloc(1, sizeof(*conf));
	if (!conf) {
		csnet_oom(sizeof(*conf));
	}
	conf->hashtbl = cs_ht_new();
	return conf;
}

void
csnet_config_free(csnet_config_t* conf) {
	cs_ht_free(conf->hashtbl);
	free(conf);
}

void
csnet_config_load(csnet_config_t* conf, const char* file) {
	FILE* f = fopen(file, "r");
	assert(f != NULL);

	char line[512] = {0};
	while (fgets(line, 512, f)) {
		if (line[0] == '#' || line[0] == '\n') {
			continue;
		}

		line[strlen(line) - 1] = '\0';
		char* p = strchr(line, '=');
		if (!p) {
			fprintf(stderr, "WARNING: `%s` line does no contain '=' in `%s`\n", line, file);
			fflush(stderr);
			continue;
		}

		*p = '\0';

		char* tmp1 = csnet_trim(line);
		char* tmp2 = csnet_trim(p + 1);
		int key_len = strlen(tmp1);
		int value_len = strlen(tmp2);
		char* key = calloc(1, key_len);
		char* value = calloc(1, value_len);

		if (!key) {
			csnet_oom(key_len);
		}

		if (!value) {
			csnet_oom(value_len);
		}

		strcpy(key, tmp1);
		strcpy(value, tmp2);
		cs_ht_insert(conf->hashtbl, key, key_len, value, value_len);
	}

	fclose(f);
}

void*
csnet_config_find(csnet_config_t* conf, void* key, int key_len) {
	if (!key) {
		return NULL;
	}

	cs_htnode_t* htnode = cs_ht_search(conf->hashtbl, key, key_len);
	if (htnode) {
		return htnode->value;
	} else {
		return NULL;
	}
}

