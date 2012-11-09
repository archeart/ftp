#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "thread.h"
#include "msg.h"

struct thread_info threads[MAX_THREAD];
extern struct sockaddr_in remote;

char* task_cache;

void thread_init()
{
	int i;
	for (i = 0; i < MAX_THREAD; i++)
		threads[i].used = 0;
	//setitimer
}

int inline get_threadd()
{
	static int i = 0;
	int j;
	if (i == 0)
		j = MAX_THREAD - 1;
	else j = i - 1;

	for (; ; i++) {
		if ( i == MAX_THREAD )
			i = 0;
		if (!threads[i].used)
			break;
		if (i == j)
			return -1;
	}

	return i;
}

void* get_thread(void*);
void* put_thread(void*);

void 
thread_create(char* name, unsigned char type, long offset)
{
	pthread_t thread;
	int i = get_threadd(), sockfd, fd;
	if (i == -1) {
		printf("Can't open more thread.\n");
//		store(name);
		return ;
	}
	else {

		if (type == CMD_PUT) {
			fd = open(name, O_RDONLY);
			if (fd < 0) {
				printf("File doesn't exist.\n");
				return ;
			}
		}
	
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			fprintf(stderr, "Can't create socket to get %s\n", name);
			return ;
		}
		if (connect(sockfd, (struct sockaddr*)&remote, sizeof(struct sockaddr)) < 0) {
			printf("Can't connect to host\n");
			close(sockfd);
			return ;
		}

		threads[i].used = 1;
		threads[i].name = name;
		threads[i].sock = sockfd;
		if (type == CMD_PUT)
			threads[i].offset = fd;
		else
			threads[i].offset = offset;

		if (type == CMD_GET)
			pthread_create(&threads[i].thread, NULL, get_thread,
				(void*)&threads[i]);

		else if (type == CMD_PUT)
			pthread_create(&threads[i].thread, NULL, put_thread,
				(void*)&threads[i]);

		
	}
}

