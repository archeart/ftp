#ifndef _AUTH_INTERFACE_H
#define _AUTH_INTERFACE_H

#include "types.h"
#include "auth_struct.h"

#define S_READ 1
#define S_WRITE 2
#define S_MKDIR 4

#define S_NO_USER -1
#define S_PASS_ERROR -2

void auth_init();

int auth_user(char *user,char *pass);

uint_8 auth_permission(char *dir,int uid); 

uint_8 auth_user_permission(char *dir,char *user);

int auth_get_uid(char *user);

int auth_delete_user(char *user);
	
#endif
