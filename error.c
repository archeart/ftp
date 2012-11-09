#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
/* for file_reply error_no */

static char* err_str[MAX_ERR];

void error_init()
{
	err_str[1] = "File does not exist.";
	err_str[2] = "Kicked out by the host.";
	err_str[3] = "You don't have the privilige.";
	err_str[4] = "The file has exists.";
	err_str[5] = "The file is not a directory.";
	err_str[6] = "The name is too long.";
	err_str[7] = "Can't rmdir.";
	err_str[8] = "Lost connection.";
}

void inline put_err(char errno)
{
	printf("%s\n", err_str[errno]);
}
