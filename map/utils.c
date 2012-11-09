#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"

bool simple_strcmp(char *a,char *b)
{
	while (*a&&*b)
	{
		if (*a!=*b) return 0;
		a++;b++;
	}

	if (*a||*b) return 0;
	return 1;
}

char simple_strcmp2(char *a,char *b)
{
	while (*a&&*b)
	{
		if (*a>*b) return 1;
		else if (*a<*b) return -1;
		
		a++;b++;
	}

	if (*a) return 1;
	else if (*b) return -1;
	else return 0;
}

char *simple_strcpy(char *a)
{
	int l=strlen(a)+1;
	int i;
	char *result;

	result=(char *)malloc(sizeof(char)*(l+1));
	for (i=0;i<l;i++)
		result[i]=a[i];
	
	return result;
}


/*
int main()
{
	char *a="nihaosa";
	char *b="nihaosa";

	char *c=simple_strcpy(a);

	printf("%s\n",c);

	return 0;
}
*/
