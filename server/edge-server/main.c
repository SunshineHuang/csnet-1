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

	struct rlimit rlimit;
	rlimit.rlim_cur = 1024 * 1000;
	rlimit.rlim_max = 1024 * 1000;
	if (setrlimit(RLIMIT_NOFILE, &rlimit)) {
		fprintf(stderr, "setrlimit RLIMIT_NOFILE failed\n");
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

	csnet_t* csnet;
	csnet_config_t* config;
	csnet_log_t* log;
	csnet_module_t* module;

	config = csnet_config_new();
	csnet_config_load(config, argv[1]);

	char* logfile;
	char* logsize;
	char* loglevel;
	char* port;
	char* maxconn;
	char* threadcount;
	char* server_connect_timeout;

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

	log = csnet_log_new(logfile, atoi(loglevel), atoi(logsize) * 1024 * 1024);
	if (!log) {
		fprintf(stderr, "can not open logfile\n");
		fflush(stderr);
		csnet_config_free(config);
		return -1;
	}

	port = csnet_config_find(config, "port", strlen("port"));
	maxconn = csnet_config_find(config, "maxconn", strlen("maxconn"));
	threadcount = csnet_config_find(config, "threadcount", strlen("threadcount"));
	server_connect_timeout = csnet_config_find(config, "server_connect_timeout", strlen("server_connect_timeout"));

	if (!port) {
		LOG_FATAL(log, "could not find `port`!");
	}

	if (!maxconn) {
		LOG_FATAL(log, "could not find `maxconn`!");
	}

	if (!threadcount) {
		LOG_FATAL(log, "could not find `threadcount`!");
	}

	if (!server_connect_timeout) {
		LOG_FATAL(log, "could not find `server_connect_timeout`!");
	}

	cs_lfqueue_t* q = cs_lfqueue_new();
	module = csnet_module_new();
	csnet_module_init(module, NULL, log, config, q);
	csnet_module_load(module, "./business_module.so");
	csnet = csnet_new(atoi(port), atoi(threadcount), atoi(maxconn), atoi(server_connect_timeout), log, module, q);
	csnet_loop(csnet, -1);

	csnet_config_free(config);
	csnet_free(csnet);
	csnet_log_free(log);

        return 0;
}

