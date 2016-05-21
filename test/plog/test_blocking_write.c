#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    
#include <fcntl.h>     
#include <sys/time.h>  
#include <sys/types.h> 
#include <sys/stat.h>
#include <syslog.h>    
#include <errno.h>     
#include <stdarg.h>    
#include <pthread.h>   

int main()
{
	int fd = open("hello.log", O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);

	int i;
	for (i = 0; i < 10000000; i++) {
		printf("3083 2014-11-13 16:41:44.13773 main.c:23 DEBUG : this is log_thread1: %d\n", i);
	}

	for (i = 0; i < 10000000; i++) {
		printf("3083 2014-11-13 16:41:44.13773 main.c:23 DEBUG : this is log_thread2: %d\n", i);
	}

	close(fd);

	return 0;
}
