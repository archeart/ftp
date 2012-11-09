#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fs_interface.h"

unsigned char buf[64];

int main(int argc,char *argv[])
{
	char *path;
	struct file_struct *fs;

	fs_init();

	if (argc!=2)
	{
		printf("Usage: ./cat filepath\n");
		return 1;
	}

	path=argv[1];

	fs=fs_open(path);
	if (fs==0) printf("%s: not exist\n",path);
	else
	{	
		int pos=0;

		if (fs->mode!=1)
		{
			printf("%s: not a regular file\n",path);
			fs_close(fs);
			return 1;
		}

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
