#include "csnet-register.h"
#include "csnet_cmd.h"
#include "csnet_head.h"
#include "csnet_pack.h"
#include "csnet_unpack.h"
#include "csnet_utils.h"
#include "csnet_sock.h"
#include "csnet_sb.h"
#include "csnet_socket_api.h"

#include <stdlib.h>
#include <string.h>

static inline int
server_type_count(char* str) {
	int n = 0;
	char* p = str;
	while (*p != '\0') {
		if (*p == ',') n++;
		p++;
	}
	return n + 1;
}

static inline void
split(char* str, int* list) {
	int i = 0;
	char *token, *string, *tofree;
	tofree = string = strdup(str);
	while ((token = strsep(&string, ",")) != NULL) {
		list[i++] = atoi(token);
	}
	free(tofree);
}

csnet_register_t*
csnet_register_new(csnet_config_t* config, csnet_log_t* log) {
	csnet_register_t* reg = calloc(1, sizeof(*reg));
	reg->log = log;

	char* mytype = csnet_config_find(config, "mytype", strlen("mytype"));
	char* myip = csnet_config_find(config, "myip", strlen("myip"));
	char* myport = csnet_config_find(config, "myport", strlen("myport"));
	char* sip = csnet_config_find(config, "sip", strlen("sip"));
	char* sport = csnet_config_find(config, "sport", strlen("sport"));
	char* accept_server_type = csnet_config_find(config, "accept-server-type", strlen("accept-server-type"));
	char* connect_server_type = csnet_config_find(config, "connect-server-type", strlen("connect-server-type"));

	if (!mytype) LOG_FATAL(log, "could not find `mytype`");
	if (!myip) LOG_FATAL(log, "could not find `myip`");
	if (!myport) LOG_FATAL(log, "could not find `myport`");
	if (!sip) LOG_FATAL(log, "could not find `sip`");
	if (!sport) LOG_FATAL(log, "could not find `sport`");

	if (accept_server_type) {
		reg->accept_server_type = strdup(accept_server_type);
		reg->accept_server_type_count = server_type_count(reg->accept_server_type);
		reg->accept_server_type_list = calloc(reg->accept_server_type_count, sizeof(int));
		split(reg->accept_server_type, reg->accept_server_type_list);
	} else {
		reg->accept_server_type_list = NULL;
		reg->accept_server_type_count = 0;
		reg->accept_server_type = NULL;
	}

	if (connect_server_type) {
		reg->connect_server_type = strdup(connect_server_type);
		reg->connect_server_type_count = server_type_count(reg->connect_server_type);
		reg->connect_server_type_list = calloc(reg->connect_server_type_count, sizeof(int));
		split(reg->connect_server_type, reg->connect_server_type_list);
	} else {
		reg->connect_server_type_list = NULL;
		reg->connect_server_type_count = 0;
		reg->connect_server_type = NULL;
	}

	reg->mytype = atoi(mytype);
	reg->myip = strdup(myip);
	reg->myport = atoi(myport);
	reg->lookup_server_fd = blocking_connect(sip, atoi(sport));
	if (reg->lookup_server_fd < 0) {
		LOG_FATAL(log, "could not connect to lookup-server[%s:%s]", sip, sport);
	}

	return reg;
}

void
csnet_register_free(csnet_register_t* reg) {
	close(reg->lookup_server_fd);
	free(reg->myip);
	if (reg->accept_server_type) {
		free(reg->accept_server_type);
	}
	if (reg->accept_server_type_list) {
		free(reg->accept_server_type_list);
	}
	if (reg->connect_server_type) {
		free(reg->connect_server_type);
	}
	if (reg->connect_server_type_list) {
		free(reg->connect_server_type_list);
	}
	free(reg);
}

int
csnet_register_myself(csnet_register_t* reg) {
	csnet_pack_t pack;
	csnet_pack_init(&pack);
	csnet_pack_puti(&pack, reg->mytype);
	csnet_pack_putstr(&pack, reg->myip);
	csnet_pack_puti(&pack, reg->myport);

	csnet_head_t h = {
		.cmd = CSNET_REGISTER_REQ,
		.status = 0x00,
		.version = VERSION,
		.compress = COMPRESS_NON,
		.ctxid = 0x00,
		.sid = 0,
		.len = HEAD_LEN + pack.len,
	};

	csnet_sock_t* sock = csnet_sock_new(1024);
	sock->fd = reg->lookup_server_fd;

	csnet_sb_t* send_buffer = csnet_sb_new(1024);
	csnet_sb_append(send_buffer, (const char*)&h, HEAD_LEN);
	csnet_sb_append(send_buffer, pack.data, pack.len);

	if (csnet_sock_send(sock, csnet_sb_data(send_buffer), csnet_sb_data_len(send_buffer)) > 0) {
		int nrecv = csnet_sock_recv(sock);
		if (nrecv < 0) {
			LOG_ERROR(reg->log, "register failed. recv() error");
			goto failed;
		}
		char* data = csnet_rb_data(sock->rb);
		csnet_head_t* head = (csnet_head_t*)data;
		if (head->cmd == CSNET_REGISTER_RSP && head->status == 0) {
			LOG_INFO(reg->log, "register done");
			csnet_sock_free(sock);
			csnet_sb_free(send_buffer);
			return 0;
		} else {
			LOG_INFO(reg->log, "register failed. status: %d", head->status);
			goto failed;
		}
	} else {
		LOG_ERROR(reg->log, "register failed: send() error");
		goto failed;
	}

failed:
	csnet_sock_free(sock);
	csnet_sb_free(send_buffer);
	return -1;
}

int
csnet_register_notice(csnet_register_t* reg) {
	if (!reg->accept_server_type) {
		LOG_WARNING(reg->log, "could not find `accept-server-type`");
		return 0;
	}

	csnet_sock_t* sock = csnet_sock_new(1024);
	sock->fd = reg->lookup_server_fd;
	csnet_sb_t* send_buffer = csnet_sb_new(1024);

	for (int i = 0; i < reg->accept_server_type_count; i++) {
		csnet_pack_t pack;
		csnet_pack_init(&pack);
		csnet_pack_puti(&pack, reg->accept_server_type_list[i]);

		csnet_head_t h = {
			.cmd = CSNET_GET_SERVERS_REQ,
			.status = 0x00,
			.version = VERSION,
			.compress = COMPRESS_NON,
			.ctxid = 0x00,
			.sid = 0,
			.len = HEAD_LEN + pack.len,
		};

		csnet_sb_append(send_buffer, (const char*)&h, HEAD_LEN);
		csnet_sb_append(send_buffer, pack.data, pack.len);

		if (csnet_sock_send(sock, csnet_sb_data(send_buffer), csnet_sb_data_len(send_buffer)) > 0) {
			int nrecv = csnet_sock_recv(sock);
			char* data = csnet_rb_data(sock->rb);
			csnet_head_t* head = (csnet_head_t*)data;

			csnet_unpack_t unpack;
			csnet_unpack_init(&unpack, data + HEAD_LEN, head->len - HEAD_LEN);

			int num = csnet_unpack_geti(&unpack);

			for (int j = 0; j < num; j++) {
				csnet_unpack_geti(&unpack);
				const char* ip = csnet_unpack_getstr(&unpack);
				int port = csnet_unpack_geti(&unpack);

				csnet_head_t h1 = {
					.cmd = CSNET_NOTICE_REQ,
					.status = 0x00,
					.version = VERSION,
					.compress = COMPRESS_NON,
					.ctxid = 0x00,
					.sid = 0,
				};

				csnet_pack_t pack1;
				csnet_pack_init(&pack1);
				csnet_pack_puti(&pack1, reg->mytype);
				csnet_pack_putstr(&pack1, reg->myip);
				csnet_pack_puti(&pack1, reg->myport);

				h1.len = HEAD_LEN + pack1.len;

				int fd = blocking_connect(ip, port);
				csnet_sock_t* sock1 = csnet_sock_new(1024);
				sock1->fd = fd;

				csnet_sb_t* send_buffer1 = csnet_sb_new(1024);
				csnet_sb_append(send_buffer1, (const char*)&h1, HEAD_LEN);
				csnet_sb_append(send_buffer1, pack1.data, pack1.len);

				csnet_sock_send(sock1, csnet_sb_data(send_buffer1), csnet_sb_data_len(send_buffer1));

				csnet_sock_recv(sock1);
				csnet_sock_free(sock1);
				close(fd);
				csnet_sb_free(send_buffer1);
			}

			csnet_rb_seek(sock->rb, nrecv);
			csnet_sb_reset(send_buffer);
		}
	}

	csnet_sock_free(sock);
	csnet_sb_free(send_buffer);
	return 0;
}

