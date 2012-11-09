#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "map/map_interface.h"
#include "auth/auth_interface.h"

#define MAX_USER 100
pthread_mutex_t user_lock;

struct user_info {
	char user[32];
	char pass[32];
	unsigned char pril;
	pthread_mutex_t lock;
	int ref;
	unsigned char used;
} user_db[MAX_USER];

int mgr_cnt;

void inline get_word(char* line, char* word) 
{
	while (*line != ' ' && *line != '\t' && *line) {
		*word = *line;
		word ++;
		line ++;
	}
	word = 0;
}

void inline get_lword(char* line, char* word)
{
	int i = strlen(line) - 1;
	while (line[i] != '/' && i >= 0) i--;
	strcpy(word, &line[i+1]);
}

void inline mgr_trim(char* buf){
	buf[strlen(buf)-1] = 0;
}

void mgr_init()
{
	int i, retval;
	FILE* fd;
	char null[10];

	memset(user_db, 0, sizeof(user_db));
	for (i = 0; i < MAX_USER; i++) 
		pthread_mutex_init(&user_db[i].lock, 0);

	auth_init();

	fd = fopen("conf/passwd", "r");
	i = 0;


	memset(user_db, 0, sizeof(user_db));
	while (1)
	{
		i++;
		if (fscanf(fd, "%s %s",user_db[i].user, user_db[i].pass) == EOF) break;
		user_db[i].used = 1;
		//printf("%s %s\n",user_db[i].user,user_db[i].pass);
	}
	fclose(fd);
	printf("end\n");
}

int inline mgr_find_name(char* name)
{
	/*
	int i;
	for (i = 0; i < MAX_USER; i++)
		if (strcmp(name, user_db[i].user) == 0)
			return i;
	return -1;
	*/

	return auth_get_uid(name);
}

void mgr_add(char* buf)
{
	/*
	char user[64];
	char pass[64];
	while (*buf == ' ' || *buf == '\t') buf++;
	get_word(buf, user);
	get_lword(buf, pass);

	if (strlen(user) > 32 || strlen(pass) > 32) {
		printf("the user name or password should be less than 32 letters\n");
		return ;
	}

	int i;
	i = mgr_find_name(user);
	if (i >= 0) {
		strcpy(user_db[i].pass, pass);
		return ;
	}

	for (i = 0; i < MAX_USER; i++) {
		if (!user_db[i].used) {
			strcpy(user_db[i].user, user);
			strcpy(user_db[i].pass, pass);
			user_db[i].ref = 0;
			user_db[i].used = 1;
		}
	}

	if (i == MAX_USER) {
		printf("There can be any more users\n");
	}
	*/
}

void mgr_del(char* buf)
{
	/*
	int i = mgr_find_name(buf);
	if (i < 0) {
		printf("there is not such user\n");
		return ;
	}
		
	mgr_kill(user);
	user_db[i].used = 0;
	pthread_mutex_lock(&user_lock);
	thread_kill(buf);
	pthread_mutex_unlock(&user_lock);
	*/
}

int mgr_cntcur(){
	int i, cnt = 0;
	for (i = 0; i < MAX_USER; i++) 
		if (user_db[i].ref > 0)
			cnt ++; 

	return cnt;	
}

int inline mgr_cntall(){
	return mgr_cnt;
}

int mgr_kill(char* name)
{
	int i = mgr_find_name(name);
	if (i >= 0) {
		pthread_mutex_lock(&user_db[i].lock);
		user_db[i].ref --;
		pthread_mutex_unlock(&user_db[i].lock);
	}
}

int mgr_check(char* name, char* pass, unsigned char first)
{
	int i = auth_user(name,pass);

	if (i>0) {
		pthread_mutex_lock(&user_db[i].lock);
		user_db[i].ref ++;
		pthread_mutex_unlock(&user_db[i].lock);
		if (first) {
			mgr_cnt ++;
			return mgr_cnt;
		}
		return 1;
	}
	return 0;
}

void mgr_statall()
{
	int i;
	for (i = 0; i < MAX_USER; i++) 
		if (user_db[i].used) 
			printf("%s has %d links now.\n", user_db[i].user, user_db[i].ref);

//	printf(">>");fflush(stdout);
}

void mgr_stat(char* name)
{
	int i = mgr_find_name(name);
	if (i == S_NO_USER) 
		printf("No such user.\n");

	printf("%s has %d links now.\n", name, user_db[i].ref);
//	printf(">>"); fflush(stdout);
	
}

