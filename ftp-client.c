#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

#include "ftp.h"
#include "msg.h"
#include "thread.h"
#include "error.h"

int errno;

int sock;
char path[128];

char username[32];
char password[32];

struct sockaddr_in remote;

/* util */
void inline trim(char *buf)
{
	int i = strlen(buf) - 1;
	while (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t') 
		buf[i--] = 0;
}

bool inline pure(char *buf)
{
	char* c = buf;
	while (*c) {
		if (*c == ' ' || *c == '\n' || *c == '\t')
			return false;
		c ++;
	}
	
	if (strlen(buf) > 31)
		return false;
	return true;
}

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

void strip(char *buf)
{
	int i, cnt = 0;

	i = strlen(buf) - 1;
	do {
		i --;
		if (i > 0 && buf[i] == '.') {
			buf[i--] = 0;
			if (buf[i] == '.') {
				cnt ++;
				buf[i--] = 0;
			}
		} else 
			break;
	} while (i > 0);

	if (cnt == 0)
		return ;

//	printf("buf:%s\n", buf);

	i = strlen(buf) - 2;
	while (i > 0 && cnt > 0) {
		while (buf[i] != '/') i --;
		cnt --;
	}
	if (i >= 0) buf[i+1] = 0;
}

bool cli_auth(int, bool);

/* ftp-command */
void inline do_close(ftp_rqt* close_rqt)
{
	send(sock, close_rqt, FTPHDR_LEN, 0);
}

void inline thread_end(struct thread_info* args)
{
	close(args->sock);
	free(args->name);
	args->used = 0;
	pthread_exit(0);
}


void* get_thread(void* arg)
{
	struct thread_info* args = arg;
	ftp_rqt request;
	struct get_request getrqt;
	struct get_reply getrpl;
	char name[32];
	int sock = args->sock, read_n, ret;
	
	if (!cli_auth(sock, false)) {
		thread_end(args);
	}

	get_lword(args->name, name);
	getrqt.offset = args->offset;
	if (args->name[0] == '/')
		strcpy(getrqt.name, args->name);
	else {
		strcpy(getrqt.name, path);
		strcat(getrqt.name, args->name);
	}

 	strcat(name, "_ftp");
	int fd = open(name, O_WRONLY);
	if (fd < 0) {
		fd = creat(name, 0666);
		getrqt.offset = 0;
	} else 
		lseek(fd, 0, SEEK_END);
		
	request.type = CMD_GET;
	memcpy(request.buf, &getrqt, sizeof(struct get_request));

	send(sock, &request, FTPHDR_LEN + GETRQT_LEN, 0);

	read_n = recv(sock, &getrpl, GETRPL_LEN, 0);
	if (getrpl.flags & GET_ERR == GET_ERR) {
		fprintf(stderr, "Error when download the file.\n");
		close(fd);
		thread_end(args);
	}
	
	while (read_n > 0) {
		ret = write(fd, getrpl.content, read_n-1);
		read_n = recv(sock, &getrpl, GETRPL_LEN, 0);

		if (read_n <= 0)
			break ;
	}

	close(fd);
	thread_end(args);
}

void* put_thread(void* arg)
{
	struct thread_info* args = arg;
	int fd = (int)args->offset;
	int sock = args->sock;
	ftp_rqt request;
	struct put_request* put_rqt;
	char reply;
	struct put_content put_con;
	struct stat fs;
	int ret, read_n, i;

	if (!cli_auth(sock, false)) {
		close(fd);
		thread_end(args);
	}

	request.type = CMD_PUT;
	put_rqt = (struct put_request*)request.buf;

	strcpy(put_rqt->path, path);
	strcpy(put_rqt->name, args->name);
	//printf("put name:%s\n", put_rqt->name);
	put_rqt->mode = 0777;

	ret = lstat(args->name, &fs);
	if (ret < 0)
		fprintf(stderr, "can't read the file's mode.\n");
	else 
		put_rqt->mode = (int)fs.st_mode;

	ret = send(sock, &request, FTPHDR_LEN + PUTRQT_LEN, 0);
	if (ret <= 0) plost;

	ret = recv(sock, &reply, PUTRPL_LEN, 0);
	if (ret <= 0) plost;
	if (reply != 0) {
		put_err(reply);
		close(fd);
		thread_end(args);
	}

	put_con.flags = 0;
	read_n = read(fd, put_con.content, 512);
	while (read_n > 0) {
		i = send(sock, &put_con, read_n+1, 0);
		if (i <= 0) {
			printf("put %s failed.\n>>", args->name);
			break ; 
		}
		read_n = read(fd, put_con.content, 512);
	}
	put_con.flags = 1;
	send(sock, &put_con.flags, 1, 0);
}

void inline do_get(char* name, long off)
{
	thread_create(name, CMD_GET, off);
}

void inline do_put(char* name)
{
	thread_create(name, CMD_PUT, 0);
}

void do_chdir(char* name, ftp_rqt* cd_rqt)
{
	fs_rqt inner;
	fs_rpl reply;
	int i;

	if (strcmp(name, ".") == 0)
		return ;

	memcpy(&inner.path, path, 128);

	if (!pure(name)) {
		fprintf(stderr, "invalid name");
		return;
	}

	memcpy(&inner.name, name, strlen(name)+1);
	memcpy(&cd_rqt->buf, &inner, FSRQT_LEN);

	i = send(sock, cd_rqt, FSRQT_LEN + FTPHDR_LEN, 0);
	if (i <= 0) kicked;

	i = recv(sock, &reply, FSRPL_LEN, 0);
	if (i == 0) {
		fprintf(stderr, "kicked out by host.\n");
	}

	if (!reply.res) {
		put_err(reply.error_no);
	} else {
		strcat(path, name);
		if (path[strlen(path)-1] != '/')
			strcat(path, "/");
		strip(path);
	}
}

void do_mkdir(char* name, ftp_rqt* rqt)
{
	fs_rqt inner;
	fs_rpl reply;
	int ret;
//	printf("%s\n", name);
	if (!pure(name)) {
		fprintf(stderr, "invalid file name\n");
		return ;
	}

	memcpy(&inner.path, path, 128);
	memcpy(&inner.name, name, strlen(name)+1);
	memcpy(&rqt->buf, &inner, FSRQT_LEN);
	ret = send(sock, rqt, FSRQT_LEN + FTPHDR_LEN, 0);
	if (ret <= 0) 
		fprintf(stderr, "kicked out by host.\n");

	ret = recv(sock, &reply, FSRPL_LEN, 0);
	if (ret == 0) {
		printf("shutdown by host.\n");
		exit(1);
	}

	if (!reply.res) 
		put_err(reply.error_no);
}

void do_rmdir(char* name, ftp_rqt* rqt)
{
	fs_rqt inner;
	fs_rpl reply;
	int ret;
	if (!pure(name)) {
		fprintf(stderr, "invalid file name\n");
		return ;
	}

	memcpy(&inner.path, path, 128);
	memcpy(&inner.name, name, strlen(name)+1);
	memcpy(&rqt->buf, &inner, FSRQT_LEN);
	ret = send(sock, rqt, FSRQT_LEN + FTPHDR_LEN, 0);
	if (ret <= 0)
		fprintf(stderr, "kicked out by host.\n");

	ret = recv(sock, &reply, FSRPL_LEN, 0);	
	if (ret == 0) {
		printf("shutdown by host.\n");
		exit(1);
	}
	if (!reply.res)
		put_err(reply.error_no);
	
}

void do_lsdir(ftp_rqt* rqt)
{
	fs_rqt inner;
	ls_rpl reply;
	int i;
	bool flag = true;

	memcpy(&rqt->buf, path, strlen(path)+1);

	i = send(sock, rqt, FSRQT_LEN + FTPHDR_LEN, 0);
	if (i <= 0)
		kicked;

	do {
		i = recv(sock, &reply, LSRPL_LEN, 0);
		if (i <= 0) kicked;

		if (reply.error_no != 0) {
			put_err(reply.error_no);
			return ;
		} else {
			if (flag) {
				printf("%32s%12s%12s\n", "name", "size", "type");
				flag = false;
			}

			for (i = 0; i < reply.cnt; i++) {
				printf("%32s%12ld", reply.item[i].name, 
					reply.item[i].size);

				if (S_ISDIR((mode_t)reply.item[i].mode))
					printf("         DIR\n");
				else
					printf("        FILE\n");
			}
		}

	} while (!reply.end);
}

/*	
 * Convert the command line to message format
 * When error occured return false
 * Otherwise return true
 */
bool parse(char* buf, ftp_rqt* request)
{

	char* name;

	while (*buf == ' ') buf++;

	/* get type */
	if (strcmp(buf, "quit") == 0) {
		request->type = CMD_CLOSE;
		do_close(request);
		return false;
	}

	else if (strcmp(buf, "lpwd") == 0) {
		printf("%s\n", path);
	}


	else if (strncmp(buf, "cd ", 3) == 0) {
		request->type = CMD_CD;
		buf += 3;
		while (*buf == ' ')
			buf ++;

		do_chdir(buf, request);
	}

	else if (strcmp(buf, "ls") == 0) {
		request->type = CMD_LS;
		buf += 2;
/*
		if (*buf != ' ')
			goto Invalid_Command;
		while (*buf == ' ')
			buf ++;
		*/
		do_lsdir(request);
	}

	else if (strncmp(buf, "mkdir", 5) == 0) {
		request->type = CMD_MKDIR;
		buf += 5;

		if (*buf != ' ')
			goto Invalid_Command;
		while (*buf == ' ')
			buf ++;

		do_mkdir(buf, request);
	}

	else if (strncmp(buf, "rmdir", 5) == 0) {
		request->type = CMD_RMDIR;
		buf += 5;

		if (*buf != ' ')
			goto Invalid_Command;
		while (*buf == ' ')
			buf ++;

		do_rmdir(buf, request);
	}

	else if (strncmp(buf, "get", 3) == 0) {
		buf += 3;
		if (*buf != ' ')
			goto Invalid_Command;
		while (*buf == ' ')
			buf ++;

		name = (char*)malloc(32);
		get_word(buf, name);
		do_get(name, 0);
		//printf("name: %s\n", name);
	}

	else if (strncmp(buf, "mget", 4) == 0) {
		buf += 4;
		if (*buf != ' ')
			goto Invalid_Command;
		while (*buf) {
			while (*buf == ' ' || *buf == '\t')
				buf ++;

			name = (char*)malloc(32);
			get_word(buf, name);
			do_get(name, 0);
			while (*buf != ' ' && *buf != '\t' && *buf)
				buf ++;
		}
	}

	else if (strncmp(buf, "put ", 4) == 0) {
		buf += 4;
		while (*buf == ' ' || *buf == '\t') buf++;
		name = (char*) malloc(32);
		strcpy(name, buf);
		do_put(name);
	}

	else if (strncmp(buf, "mput ", 5) == 0) {
		buf += 5;
		while (*buf) {
			while (*buf == ' ' || *buf == '\t')
				buf ++;
			
			name = (char*) malloc(32);
			get_word(buf, name);
			do_put(name);

			while (*buf != ' ' && *buf != '\t' && *buf)
				buf ++;
		}
	}

	else {
		goto Invalid_Command;
	}	

	return true;

Invalid_Command:
	fprintf(stderr, "No such command.\n");
	printf("press command \'help\' for more info:\n");
	//command_list;
	return true;
}


bool cli_auth(int sock, bool first)
{
	struct auth auth_msg;
	int reply;
	int retval;
	strcpy(auth_msg.user, username);
	strcpy(auth_msg.pass, password);
	auth_msg.first = first;

	retval = send(sock, &auth_msg, sizeof(struct auth), 0);
//	printf("%s %s %d\n", username, password, retval);

	int ret = recv(sock, &reply, sizeof(int), 0);
	if (ret <= 0 || !reply) {
		fprintf(stderr, "Error when auth\n");
		printf("user doesn't exist or password dismatch.\n");
		return false;
	}
	if (first)
		printf("You are the #%d visitor\n", reply);
	return true;
}

void init()
{
	error_init();
	thread_init();

	printf("please enter you username: ");
	fgets(username, 32, stdin);
	trim(username);
	printf("please enter you password: ");
	fgets(password, 32, stdin);
	trim(password);
//	printf("%s\n%s\n", username, password);
	if (!cli_auth(sock, true))
		exit(1);
}

int main()
{
	socklen_t addr_size = sizeof(struct sockaddr);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(PORT);
	remote.sin_addr.s_addr = inet_addr(HOST);

	if (connect(sock, (struct sockaddr*)&remote, addr_size) < 0) {
		printf("error when connect\n");
		exit(1);
	}


	char buf[256];
	ftp_rqt request;

/*	
 *  Don't forget to close the sock when an 
 *  "Error" happens
 */

	init();
	memcpy(path, "/", 2);
	while (true) {
		printf("[%s@ftp] %s $ ",username,path);
		fgets(buf, 256, stdin);

		trim(buf);
		if (!parse(buf, &request))
			break;
	}

	close(sock);
	return 0;
}
