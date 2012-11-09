#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "types.h"
#include "fs_struct.h"

struct file_struct *files=0;

pthread_mutex_t lock;

void fs_init()
{
	printf("*** FS SYSTEM INIT start ***\n");

	pthread_mutex_init(&lock,0);

	printf("*** FS SYSTEM INIT COMPLETE ***\n\n");
}

static struct file_struct *fs_alloc()
{
	MALLOC(struct file_struct,block);
	
	if (block==0) return 0;

	block->used=1;
	block->next=0;
	return block;
}

static struct file_struct *fs_find(char *path)
{
	return 0;
}

struct file_struct *fs_open(char *path)
{
	FILE *fp;
	struct file_struct *block;
	int i;

	pthread_mutex_lock(&lock);

	block=fs_find(path);
	if (block!=0)
	{
		block->used++;

		pthread_mutex_unlock(&lock);
		return block;
	}

	fp=fopen(path,"rb");
	if (fp==0) 
	{
		pthread_mutex_unlock(&lock);
		return 0;
	}

	block=fs_alloc();

	if (block!=0)
	{
		struct stat sb;
		int l;

		block->fp=fp;

		stat(path,&sb);
		block->size=sb.st_size;

		l=strlen(path);
		bcopy(path,block->path,l+1);

		block->mode=!!(sb.st_mode&S_IFREG);

		block->next=files;
		files=block;
	}

	pthread_mutex_unlock(&lock);
	return block;
}

int fs_read(struct file_struct *fs,uint_32 start,uint_32 len,uint_8 *buf)
{
	fseek(fs->fp,start,SEEK_SET);
	return fread(buf,1,len,fs->fp);
}

void fs_close(struct file_struct *fs)
{
	pthread_mutex_lock(&lock);

	fs->used--;
	if (fs->used==0)
	{
		fclose(fs->fp);
	}

	pthread_mutex_unlock(&lock);
}

/*
uint_8 buf[64];

int main()
{
	char *path="fs_struct.h";
	struct file_struct *fs;

	fs=fs_open(path);
	if (fs==0) printf("not exist\n");
	else
	{	
		int pos=0;
		printf("%s,%d,%d\n",fs->path,fs->used,fs->size);

		while (1)
		{
			int nbytes=fs_read(fs,pos,63,buf);
			pos+=nbytes;

			buf[nbytes]='\0';
			printf("%s",buf);

			if (nbytes<63) break;
		}
	}
	return 0;
}
*/
