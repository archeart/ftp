#ifndef _FS_INTERFACE_H
#define _FS_INTERFACE_H

#include "types.h"
#include "fs_struct.h"

/* before use the fs system, fs_init() must be called */
void fs_init();

/* open a file, return a file_struct */
struct file_struct *fs_open(char *path);

/* read a file, need file_struct/begin position/length want to read/buf's address */
int fs_read(struct file_struct *fs,uint_32 start,uint_32 len,uint_8 *buf);

/* close a file */
void fs_close(struct file_struct *fs);
	
#endif
