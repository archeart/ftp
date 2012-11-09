#define MAX_THREAD 10

struct thread_info {
	pthread_t thread;
	unsigned char used;
	char *name;
	long offset;
	int sock;
}; 

#define THREAD_INFO_LEN sizeof(struct thread_info)



void thread_create(char*, unsigned char, long);
