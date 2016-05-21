#include "plog.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char** argv)
{
	struct plog* plog;
	plog = plog_create("./log", PLOG_LEVEL_DEBUG, 1024 * 1024 * 5);
	PLOG_DEBUG(plog, "plog debug");
	PLOG_INFO(plog, "plog info");
	PLOG_WARNING(plog, "plog warning");
	PLOG_ERROR(plog, "plog error");

	sleep(5);

	plog_destroy(plog);

	return 0;
}

