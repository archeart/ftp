#ifndef _FS_STRUCT_H
#define _FS_STRUCT_H

#define MAX_PATH_LEN 255
#define MAX_FILE 255

struct file_struct
{
	int used;
	FILE *fp;
	int size;
	int mode;
	struct file_struct *next;
	char path[MAX_PATH_LEN+1];
};


#endif
