#ifndef _AUTH_STRUCT_H
#define _AUTH_STRUCT_H

#define USERNAME_LEN 255
#define PASSWORD_LEN 255
#define DIR_LEN 255

struct auth_profile
{
	int uid;
	char pass[PASSWORD_LEN+1];
};

#endif
