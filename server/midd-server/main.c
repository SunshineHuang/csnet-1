#include "libcsnet.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>

int main(int argc, char** argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s config\n", argv[0]);
		fflush(stderr);
		return -1;
	}

	char* conf_file = argv[1];

	struct rlimit rlimit;
	rlimit.rlim_cur = 1024 * 1000;
	rlimit.rlim_max = 1024 * 1000;
	if (setrlimit(RLIMIT_NOFILE, &rlimit)) {
		fprintf(stderr, "setrlimit(RLIMIT_NOFILE) failed: %s\n", strerror(errno));
		fflush(stderr);
		return -1;
	}

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	signal(SIGPIPE, SIG_IGN);  /* avoid send() crashes */
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	/*
	daemon(1, 1);
	*/

	char* logfile;
	char* logsize;
	char* loglevel;
	char* port;
	char* maxconn;
	char* threadcount;
	char* server_connect_timeout;
	char* client_connect_timeout;
	char* business_timeout;

	csnet_config_t* config;
	csnet_log_t* logger;
	csnet_t* csnet;
	cs_lfqueue_t* q;
	csnet_module_t* module;
	csnet_conntor_t* conntor;
	csnet_ctx_t* ctx;
	csnet_hotpatch_t* hotpatch;

	config = csnet_config_new();
	csnet_config_load(config, conf_file);

	logfile = csnet_config_find(config, "logfile", strlen("logfile"));
	logsize = csnet_config_find(config, "logsize", strlen("logsize"));
	loglevel = csnet_config_find(config, "loglevel", strlen("loglevel"));

	if (!logfile) {
		fprintf(stderr, "could not find `logfile`!");
		fflush(stderr);
		csnet_config_free(config);
		return -1;
	}

	if (!logsize) {
		fprintf(stderr, "could not find `logsize`!");
		fflush(stderr);
		csnet_config_free(config);
		return -1;
	}

	if (!loglevel) {
		fprintf(stderr, "could not find `loglevel`!");
		fflush(stderr);
		csnet_config_free(config);
		return -1;
	}

	logger = csnet_log_new(logfile, atoi(loglevel), atoi(logsize) * 1024 * 1024);

	 if (!logger) {
		 fprintf(stderr, "can not open logfile\n");
		 fflush(stderr);
		 csnet_config_free(config);
		 return -1;
	}

	port = csnet_config_find(config, "port", strlen("port"));
	maxconn = csnet_config_find(config, "maxconn", strlen("maxconn"));
	threadcount = csnet_config_find(config, "threadcount", strlen("threadcount"));
	server_connect_timeout = csnet_config_find(config, "server_connect_timeout", strlen("server_connect_timeout"));
	client_connect_timeout = csnet_config_find(config, "client_connect_timeout", strlen("client_connect_timeout"));
	business_timeout = csnet_config_find(config, "business_timeout", strlen("business_timeout"));

	if (!port) {
		LOG_FATAL(logger, "could not find `port`!");
	}

	if (!maxconn) {
		LOG_FATAL(logger, "could not find `maxconn`!");
	}

	if (!threadcount) {
		LOG_FATAL(logger, "could not find `threadcount`!");
	}

	if (!server_connect_timeout) {
		LOG_FATAL(logger, "could not find `server_connect_timeout`!");
	}

	if (!client_connect_timeout) {
		LOG_FATAL(logger, "could not find `client_connect_timeout`!");
	}

	if (!business_timeout) {
		LOG_FATAL(logger, "could not find `business_timeout`!");
	}

 	q = cs_lfqueue_new();
	ctx = csnet_ctx_new(CTX_SIZE, atoi(business_timeout), q);
	module = csnet_module_new();
	conntor = csnet_conntor_new(atoi(client_connect_timeout), conf_file, logger, module, q);
	csnet_module_init(module, conntor, logger, ctx, q);
	csnet_module_load(module, "./business_module.so");
	csnet_conntor_connect_servers(conntor);
	csnet_conntor_loop(conntor);
	csnet = csnet_new(atoi(port), atoi(threadcount), atoi(maxconn), atoi(server_connect_timeout), logger, module, q);
	hotpatch = csnet_hotpatch_new(q, csnet, logger, ctx, conntor, module);
	LOG_INFO(logger, "Server start ok ...");
	csnet_loop(csnet, -1);

	cs_lfqueue_free(q);
	csnet_config_free(config);
	csnet_module_free(module);
	csnet_conntor_free(conntor);
	csnet_log_free(logger);
	csnet_free(csnet);
	csnet_ctx_free(ctx);
	csnet_hotpatch_free(hotpatch);

        return 0;
}

