#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>

#include "ftp.h"
#include "msg.h"
#include "sthread.h"
#include "user_mgr.h"
#include "serror.h"
#include "fs/fs_interface.h"
#include "map/map_interface.h"
#include "auth/auth_interface.h"

#define logback 100
#define root "share"

struct thread_args{
	int sockfd;
	struct sockaddr_in client;
};

/* util */
void inline trim(char *buf)
{
	int i = strlen(buf) - 1;
	while (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t') 
		buf[i--] = 0;
}

/* */
void do_chdir(ftp_rqt* request, int sock)
{
	fs_rqt cd_rqt;
	fs_rpl rpl;
	struct stat fs;
	char name[128];
	int retval;

	memcpy(&cd_rqt, &request->buf, FSRQT_LEN);
//	printf("cd %s%s\n", cd_rqt.path, cd_rqt.name);

	strcpy(name, root);
	strcat(name, cd_rqt.path);
	strcat(name, cd_rqt.name);

	retval = lstat(name, &fs);
	if (retval != 0) {
		rpl.res = false;
		rpl.error_no = E_NEXIST;
		send(sock, &rpl, FSRPL_LEN, 0);
		return ;
	}
	if (!S_ISDIR(fs.st_mode)) {
		rpl.res = false;
		rpl.error_no = E_NDIR;
	} else {
		rpl.res = true;
	//	printf("retval: %d; chdir: %s\n", retval, name);
	}

	send(sock, &rpl, FSRPL_LEN, 0);
}

void do_mkdir(ftp_rqt* request, int sock)
{
	fs_rqt rqt;
	fs_rpl rpl;
	char path[128];
	int retval;

	memcpy(&rqt, &request->buf, FSRQT_LEN);
//  printf("mkdir share%s%s\n", rqt.path, rqt.name);

	memset(&rpl, 0, sizeof(fs_rpl));
	rpl.res = true;

	if (strlen(rqt.path) + strlen(rqt.name) > 128) {
		rpl.res = false;
		rpl.error_no = E_NAME;
	}

	strcpy(path, root);
	strcat(path, rqt.path);
	strcat(path, rqt.name);

	retval = mkdir(path, 0777);
	if (retval < 0) {
		rpl.res = false;
		rpl.error_no = E_EXIST;
		//rpl.errorno = 
	}

	send(sock, &rpl, FSRPL_LEN, 0);
}

void do_rmdir(ftp_rqt* request, int sock)
{
	fs_rqt rqt;
	fs_rpl rpl;
	char path[128];
	int retval;
	memcpy(&rqt, &request->buf, FSRQT_LEN);
//	printf("rmdir share%s%s\n", rqt.path, rqt.name);
	memset(&rpl, 0, sizeof(fs_rpl));

	strcpy(path, root);
	strcat(path, rqt.path);
	strcat(path, rqt.name);
	retval = rmdir(path);
	rpl.res = true;
	if (retval < 0) {
		rpl.res = false;
		rpl.error_no = E_RMDIR;
	}
	send(sock, &rpl, FSRPL_LEN, 0);
}

void do_lsdir(ftp_rqt* request, int sock)
{
	struct stat fs;
	char path[128], name[128];
	int retval;
	ls_rpl rpl;
	DIR *dir;
	struct dirent* entry;

	strcpy(path, root);
	strcat(path, request->buf);
	//printf("ls %s\n", path);

	retval = lstat(path, &fs);

	rpl.error_no = 0;		
	if (!S_ISDIR(fs.st_mode)) {
//		rpl.error_no
		rpl.error_no = -1;
		send(sock, &rpl, 0, LSRPL_LEN);
		return ;
	}

	dir = opendir(path);
	rpl.cnt = 0;
	rpl.end = false;

	short cnt = 1;

	entry = readdir(dir);
	while (entry != NULL) {
		strcpy(name, path);
		strcat(name, entry->d_name);
		lstat(name, &fs);

		strcpy(rpl.item[rpl.cnt].name, entry->d_name);
		rpl.item[rpl.cnt].size = (long)fs.st_size;
		rpl.item[rpl.cnt].mode = (int) fs.st_mode;

		rpl.cnt ++;
		entry = readdir(dir);
		if (rpl.cnt == 32 && entry != NULL) {
			send(sock, &rpl, LSRPL_LEN, 0);
			rpl.cnt = 0;
		}
	}

	rpl.end = true;
	send(sock, &rpl, LSRPL_LEN, 0);
}

void do_get(ftp_rqt* request, int sock)
{
	struct get_request* getrqt = (struct get_request*)request->buf;
	struct get_reply getrpl;
	char buf[512];
	int block_size=256;
	int read_n, ret;
	struct file_struct *fs;

	printf("get %s\n", getrqt->name);
	strcpy(buf, root);
	strcat(buf, getrqt->name);

	memset(&getrpl, 0, GETRPL_LEN);

	fs=fs_open(buf);
	if (fs == 0) 
	{
		getrpl.flags = GET_ERR;
		fprintf(stderr, "Error when open %s\n", buf);
		send(sock, &getrpl, 1, 0);
		return ;
	}
	else
	{
		int pos=getrqt->offset;

		while (1)
		{
			int nbytes=fs_read(fs,pos,block_size,buf);
			pos+=nbytes;

			//buf[nbytes]='\0';
			//printf("%s",buf);

			getrpl.flags = 0;
			memcpy(getrpl.content, buf, nbytes);
			ret = send(sock, &getrpl, nbytes+1, 0);
			
			if (ret <= 0) 
			break ; 

			if (nbytes<block_size) break;
		}
		fs_close(fs);
	}

	/*
	int fd = open(buf, O_RDONLY);
	if (fd < 0) {
		getrpl.flags = GET_ERR;
		fprintf(stderr, "Error when open %s\n", buf);
		send(sock, &getrpl, 1, 0);
		return ;
	}

	lseek(fd, getrqt->offset, SEEK_SET);

	read_n = read(fd, buf, 512);
	while (read_n > 0) {
		getrpl.flags = 0;
		memcpy(getrpl.content, buf, 512);
		ret = send(sock, &getrpl, read_n+1, 0);
		read_n = read(fd, buf, 512);
		if (ret <= 0) 
			break ; 
	}
	close(fd);
	*/
}

void do_put(ftp_rqt* request, int sock)
{
	struct put_request* put_rqt;
	struct put_reply put_rpl;
	struct put_content put_con;
	int fd, i, read_n;
	char name[64];

	put_rqt = (struct put_request*)request->buf;
//	printf("%s%s %x\n", put_rqt->path, put_rqt->name, put_rqt->mode);

	strcpy(name, root);
	strcat(name, put_rqt->path);
	strcat(name, put_rqt->name);

	fd = open(name, O_RDONLY);
	if (fd >= 0) {
		close(fd);
		put_rpl.flags = E_EXIST;
		send(sock, &put_rpl, PUTRPL_LEN, 0);
		return ;
	}

	put_rpl.flags = 0;
	i =	send(sock, &put_rpl, PUTRPL_LEN, 0);
	if (i <= 0) {
		fprintf(stderr, "lost connection.");
		return ;
	}

	fd = creat(name, (mode_t)put_rqt->mode);
	read_n = recv(sock, &put_con, PUTCON_LEN, 0);
	while (!put_con.flags) {
		i = write(fd, put_con.content, read_n-1);
		read_n = recv(sock, &put_con, PUTCON_LEN, 0);
		if (read_n == 0) {
			break; 
		}
	}
	close(fd);
}

void inline thread_end(struct thread_info* args)
{
	char* name;
	name = (char*)malloc(32);
	strcpy(name, args->user);
	close(args->sock);

	args->used = 0;
	mgr_kill(name);
	pthread_exit(0);
}

void* func_msg(void* arg)
{
	struct thread_info* args = arg;
//	struct sockaddr_in client;
	int sock = args->sock;
	int read_n, errcnt = 0;
	ftp_rqt request;


//	printf("in thread\n");
//	fprintf(stderr, "sock: %d\n", sock);
	while (true) {

		memset(&request, 0, RQT_LEN);
		read_n = recv(sock, (char*)&request, RQT_LEN, 0);

		if (read_n < 0) {
			errcnt ++;
			if (errcnt > 10)
				break ;
			continue ;
		}
		if (read_n == 0) {
			fprintf(stderr, "quit");
			break ; 
		}

	//	printf("msg: %d; read_n: %d\n", request.type, read_n);

		if (request.type == CMD_CLOSE)
			break;

		else if (request.type == CMD_CD) 
			do_chdir(&request, sock);

		else if (request.type == CMD_MKDIR)
			do_mkdir(&request, sock);

		else if (request.type == CMD_RMDIR)
			do_rmdir(&request, sock);

		else if (request.type == CMD_LS)
			do_lsdir(&request, sock);

		else if (request.type == CMD_GET) {
			do_get(&request, sock);
			break;
		}

		else if (request.type == CMD_PUT) {
			printf("put\n");
			do_put(&request, sock);
			break;
		}

//		send(sock, CONFIRM, strlen(CONFIRM)+1, 0);
	}

	thread_end(args);
}

/* user interface */
void* server_mgr(void* args)
{
	char buf[128];

	while (1) {
		printf(">>");
		fgets(buf, 128, stdin);
		trim(buf);
		if (strncmp(buf, "add ", 4) == 0)
			mgr_add(buf+4);

		else if (strcmp(buf, "count current") == 0) {
			printf("There are %d user(s) now.\n", mgr_cntcur());
		}
		else if (strcmp(buf, "count all") == 0) {
			printf("There are %d visitor(s) in history.\n", mgr_cntall());
		}
		else if (strncmp(buf, "kill ", 5) == 0)
			thread_kill(buf+5);
		else if (strcmp(buf, "stat all") == 0)
			mgr_statall();
		else if (strncmp(buf, "stat ", 5) == 0)
			mgr_stat(buf+5);

		else 
			printf("Error Command\n");

	}
}

pthread_t thread;
void init()
{
	map_init();
	mgr_init();
	fs_init();
	pthread_create(&thread, NULL, server_mgr, NULL);
	thread_init();
}

int main(int argc, char* argv[])
{
	int ser_sk = socket(AF_INET, SOCK_STREAM, 0), sock, ret;
	struct sockaddr_in host, remote;
	struct thread_args *args;
	char buf[256];
	socklen_t addr_size;

	init();

	if (ser_sk == -1) {
		fprintf(stderr, "Can't create the socket\n");
		exit(1);
	}
	
	addr_size = sizeof(struct sockaddr);
	memset(&host, 0, sizeof(struct sockaddr));
	memset(&remote, 0, sizeof(struct sockaddr));

	int yes=1;

	if (setsockopt(ser_sk,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)
	{
		perror("setsockopt");
		return 0;
	}

	host.sin_family = AF_INET;
	host.sin_port = htons(PORT);
	host.sin_addr.s_addr = inet_addr(HOST);

	int res;
	if ((res = bind(ser_sk, (struct sockaddr*)&host, addr_size)) < 0) {
		fprintf(stderr, "Error when binding socket with host\n");
		exit(1);
	}
	
	if (listen(ser_sk, logback) < 0) {
		fprintf(stderr, "Error when listening");
		exit(1);
	}

	while (true) {
		sock = accept(ser_sk, (struct sockaddr*)&remote, &addr_size);
		if (sock < 0) {
			fprintf(stderr, "can't accept\n");
			continue;
		}
		
		fprintf(stderr, "\nGet connection from: %s:%d\n", 
			(char*)inet_ntoa(remote.sin_addr), htons(remote.sin_port));
		
		printf(">>");fflush(stdout);

		ret = thread_create(sock, &remote);
		if (ret > 0) { 
			close(sock);
			fprintf(stderr, "can't create thread.\n");
		}
	}

	close(sock);
	close(ser_sk);
	return 0;
}
