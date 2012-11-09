#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "sthread.h"
#include "msg.h"



struct thread_info threads[MAX_THREAD];


void thread_init()
{
	memset(threads, 0, sizeof(threads));
}	

extern pthread_mutex_t user_lock;

unsigned char cli_auth(int sock, struct thread_info* task)
{
/* check password & name
 * if failed return false
 */
 	struct auth auth_msg;
	int reply;
 	recv(sock, &auth_msg, sizeof(struct auth), 0);

	reply = mgr_check(auth_msg.user, auth_msg.pass, auth_msg.first);
	send(sock, &reply, sizeof(int), 0);
	if (reply)
		strcpy(task->user, auth_msg.user);

	return reply;
}

int inline get_threadd()
{
	static int i = 0;
	int j;

	if (i > 0) j = i - 1;
	else j = MAX_THREAD - 1;
	
	for (; ; i++) {
		if (i == MAX_THREAD)
			i = 0;
		if (!threads[i].used)
			break;
		if (i == j)
			return -1;
	}

	return i;
}

void* func_msg(void*);

unsigned char thread_create(int sock, struct sockaddr_in* remote)
{	
	int ret;
	pthread_mutex_lock(&user_lock);

	int i = get_threadd();
	if (i < 0) {
		pthread_mutex_unlock(&user_lock);
		return 1;
	}

	ret = cli_auth(sock, &threads[i]);
	if (!ret) {
		close(sock);
		pthread_mutex_unlock(&user_lock);
		return 0;
	}

	threads[i].used = 1;
	threads[i].sock = sock;
	threads[i].sig = 0;
	memcpy(&threads[i].client, (char*)remote, sizeof(struct sockaddr));
	
	if (pthread_create(&threads[i].thread, 0, func_msg, (void*)&threads[i]) < 0) {
		mgr_kill(threads[i].user);
		threads[i].used = 0;
		pthread_mutex_unlock(&user_lock);
		return 2;
	}
	pthread_mutex_unlock(&user_lock);
	return 0;
}

void thread_kill(char* buf)
{
	int i;
	for (i = 0; i < MAX_THREAD; i++)
		if (threads[i].used && strcmp(buf, threads[i].user) == 0) {
			//threads[i].sig = 1;
			pthread_cancel(threads[i].thread);
			close(threads[i].sock);
			threads[i].used = 0;
			mgr_kill(buf);
		}
}

void inline lock_ti(struct thread_info* task)
{
/* this lock can only satisfy here */
	while (task->lock) pthread_yield();
	task->lock = 1;
}

void inline unlock_ti(struct thread_info* task)
{
	task->lock = 0;
}
