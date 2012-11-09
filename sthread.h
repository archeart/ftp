
#include <netinet/in.h>

#define MAX_THREAD 100

struct thread_info {
	pthread_t thread;
	char user[32];			/* CRITICAL SECTION */
	unsigned char used;
	unsigned char lock;
	int sock;
	struct sockaddr_in client;
	unsigned char sig;
};

unsigned char thread_create(int, struct sockaddr_in*);
void thread_init();



