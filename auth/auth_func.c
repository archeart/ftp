#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "auth_interface.h"
#include "auth_utils.h"
#include "../map/map_interface.h"

static pthread_mutex_t auth_lock;
char *passwd_file="conf/passwd";
char *permission_file="conf/permission";

static int uid_alloc()
{
	static int uid=0;
	
	uid++;
	return uid;
}

static struct auth_profile *get_profile_by_name(char *uname)
{

	int i;

	i=map_find(uname);

	if (i==MAP_NOT_FOUND) return 0;
	else return (struct auth_profile *)i;
}

int auth_delete(char *user)
{
	struct auth_profile *p=get_profile_by_name(user);

	if (p!=0)
	{
		map_delete(user);

		//struct auth_profile need to be free
		return p->uid;
	}
	return S_NO_USER;
}

static int get_uid_by_name(char *uname)
{
	struct auth_profile *p=get_profile_by_name(uname);

	if (p==0) return S_NO_USER;
	else return p->uid;
}

static uint_8 give_permission_mask(char *per)
{
	uint_8 mask=0;

	for (;*per;per++)
	{
		if (*per=='r') mask|=S_READ;
		else if (*per=='w') mask|=S_WRITE;
		else if (*per=='m') mask|=S_MKDIR;
	}
	return mask;
}

static char *storage(char *dir,int uid)
{
	dir[0]=(char)uid+48;
	return dir;
}

static void fix_dir(char *dir)
{
	int l=strlen(dir);

	if (l>3&&dir[l-2]=='/') l--;
	dir[l-1]='\0';
}

void auth_init()
{
	FILE *fp;
	static char uname[USERNAME_LEN+1];
	static char dir[DIR_LEN+1];
	static char per[DIR_LEN+1];

	printf("*** AUTH SYSTEM INIT START ***\n");
	pthread_mutex_init(&auth_lock,0);

	fp=fopen(passwd_file,"r");
	if (fp==0) printf("passwd NOT EXISTS\n");
	else
	{
		//printf("USER:\n");
		while (!feof(fp))
		{
			MALLOC(struct auth_profile,p);

			fscanf(fp,"%s",uname);
			if (feof(fp)) break;

			fscanf(fp,"%s",p->pass);
			
			p->uid=uid_alloc();

			map_insert(uname,(int)p);
			//printf("%s %s\n",uname,p->pass);
		}

		printf("passwd READ COMPLETE\n");
		fclose(fp);
	}

	fp=fopen(permission_file,"r");
	if (fp==0) printf("permission NOT EXISTS\n");
	else
	{
		while (!feof(fp))
		{
			fscanf(fp,"%s",dir);
			if (feof(fp)) break;
			
			fix_dir(dir);

			while (1)
			{
				int uid;

				fscanf(fp,"%s",uname);
				if (auth_strcmp(uname,"[end]")) break;

				fscanf(fp,"%s",per);
				
				uid=get_uid_by_name(uname);
				if (uid>0)
				{
					uint_8 mask=give_permission_mask(per);

					map_insert(storage(dir,uid),mask);
					//printf("DIR:%s %s %x\n",dir,per,mask);
				}
			}
		}
		printf("permission READ COMPLETE\n");
		fclose(fp);
	}
	printf("*** AUTH SYSTEM INIT COMPLETE ***\n\n");
}

int auth_user(char *user,char *pass)
{
	struct auth_profile *p=get_profile_by_name(user);

	if (p==0) return S_NO_USER;
	else if (!auth_strcmp(p->pass,pass)) return S_PASS_ERROR;
	else return p->uid;
}

uint_8 auth_permission(char *dir,int uid)
{
	uint_8 mask=0;
	int l,i;
	static char p_dir[DIR_LEN+1];

	pthread_mutex_lock(&auth_lock);
	
	l=strlen(dir);

	bcopy(dir,p_dir+1,l+1);
	storage(p_dir,uid);
	l++; 

	i=map_find(p_dir);
	printf("%s\n",p_dir);
	
	do	
	{
		int i;
		
		while (l>3&&p_dir[l-1]!='/') l--;
		p_dir[l-1]='\0';
		l--;

		if (l>1)
		{
			i=map_find(p_dir);
			printf("%s\n",p_dir);
			if (i!=MAP_NOT_FOUND) mask|=(uint_8)i;
		}
	} while (l>2);

	pthread_mutex_unlock(&auth_lock);

	return mask;
}

uint_8 auth_user_permission(char *dir,char *user)
{
	int uid;

	uid=get_uid_by_name(user);
	if (uid==S_NO_USER||uid==S_PASS_ERROR) return 0;
	else return auth_permission(dir,uid);
}

int auth_get_uid(char *user)
{
	return get_uid_by_name(user);
}
/*
int main()
{
	map_init();
	auth_init();

	printf("%d\n",get_uid_by_name("zwz"));
	printf("%d\n",get_uid_by_name("zwzmzd"));
	printf("%d\n",get_uid_by_name("nishi"));

	printf("%d\n",auth_user("zwz","aaa"));
	printf("%d\n",auth_user("iii","aaa"));
	printf("%d\n",auth_user("zwz","mzd"));

	auth_permission("/pro",1);
	auth_permission("/",1);
	auth_permission("/a/a/a/a/a",1);
	printf("%x\n",auth_permission("/pp",1));
	return 0;
}
*/
