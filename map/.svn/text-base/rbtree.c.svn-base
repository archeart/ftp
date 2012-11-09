#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "struct.h"
#include "utils.h"
#include "map_interface.h"

static struct rbt_struct *head=0;
static pthread_mutex_t map_lock;

void map_init()
{
	printf("*** MAP SYSTEM INIT START ***\n");

	pthread_mutex_init(&map_lock,0);
	
	printf("*** MAP SYSTEM INIT COMPLETE ***\n\n");
}

static void swap(void *a,void *b)
{
	static unsigned char pool[100];
	
}
static void color_flip(struct rbt_struct *root)
{
	root->type=R;
	root->left->type=B;
	root->right->type=B;
}

static void cc3_repair_left(struct rbt_struct **ref)
{
	struct rbt_struct *root=*ref;
	struct rbt_struct *a=root,*b=root->left,*c;
	
	if (b->right&&b->right->type==R)
	{
		c=b->right;
		
		b->right=c->left;
		a->left=c->right;
		
		c->left=b;
		c->right=a;
		
		c->type=B;
		a->type=R;
		
		*ref=c;
	}
	else
	{
		c=b->left;
		
		a->left=b->right;
		b->right=a;
		
		b->type=B;
		a->type=R;
		
		*ref=b;
	}
}

static void cc3_repair_right(struct rbt_struct **ref)
{
	struct rbt_struct *root=*ref;
	struct rbt_struct *a=root,*b=root->right,*c;
	
	if (b->left&&b->left->type==R)
	{
		c=b->left;
		
		b->left=c->right;
		a->right=c->left;
		
		c->right=b;
		c->left=a;
		
		c->type=B;
		a->type=R;
		

		(*ref)=c;
	}
	else
	{
		c=b->right;
		
		a->right=b->left;
		b->left=a;
		
		b->type=B;
		a->type=R;
		
		*ref=b;
	}
}

static int repair_left(int status,struct rbt_struct **ref)
{
	struct rbt_struct *root=*ref;
	
	if (status==OK) return OK;
	
	if (status==BRB)
	{
		if (root->type==B) return OK;
		else
		{
			return RRB;
		}
	}
	else if (root->right==0||root->right->type==B)
	{
		cc3_repair_left(ref);
		return OK;
	}
	else
	{
		color_flip(root);
		return BRB;
	}
}

static int repair_right(int status,struct rbt_struct **ref)
{
	struct rbt_struct *root=*ref;
	
	if (status==OK) return OK;
	
	if (status==BRB)
	{
		if (root->type==B) return OK;
		else
		{
			return RRB;
		}
	}
	else if (root->left==0||root->left->type==B)
	{
		cc3_repair_right(ref);
		return OK;
	}
	else
	{
		color_flip(root);
		return BRB;
	}
}

static int insert(struct rbt_struct **ref,char *key,int val)
{	
	if (*ref==0)
	{
		MALLOC(struct rbt_struct,node);
		
		node->left=0;
		node->right=0;
		node->type=ref==&head?B:R;

		node->key=simple_strcpy(key);
		node->val=val;
		
		*ref=node;
		return BRB;
	}
	else
	{
		int status;
		struct rbt_struct *root=*ref;
		int comp;

		comp=simple_strcmp2(key,root->key);
		
		if (comp==-1)
		{
			status=insert(&root->left,key,val);
			return repair_left(status,ref);
		}
		else if (comp==1)
		{
			status=insert(&root->right,key,val);
			status=repair_right(status,ref);
			return status;
		}
		else
		{
			root->val=val;
			printf("*** FATAL ERROR: THE SAME KEY ***\n");
			return OK;
		}
	}
}

void map_insert(char *key,int val)
{
	pthread_mutex_lock(&map_lock);

	insert(&head,key,val);
	head->type=B;

	pthread_mutex_unlock(&map_lock);
}

void rbt_delete_item(struct rbt_struct **head_ref,char *key);

void map_delete(char *key)
{
	pthread_mutex_lock(&map_lock);

	rbt_delete_item(&head,key);
	
	pthread_mutex_unlock(&map_lock);
}


static void print(struct rbt_struct *root,int deep)
{
	int i;
	
	if (!root) return;
	for (i=0;i<deep;i++) printf("    ");
	printf("%s %d %d %x %x(%x)\n",root->key,root->val,root->type,root->left,root->right,(unsigned int)root);
	print(root->left,deep+1);
	print(root->right,deep+1);
}

void preorder(struct rbt_struct *root)
{
	if (!root) return;
	preorder(root->left);
	printf("%s %d\n",root->key,root->val);
	preorder(root->right);
}


static int black_height(struct rbt_struct *root)
{
	int l,r;

	if (root==0) return 1;

	l=black_height(root->left);
	if (l==0) return 0;

	r=black_height(root->right);
	if (r==0) return 0;

	if (l!=r) return 0;
	
	return l+root->type;
}

int map_find(char *key)
{
	struct rbt_struct *p;

	pthread_mutex_lock(&map_lock);

	p=head;
	while (p!=0)
	{
		int comp=simple_strcmp2(key,p->key);

		if (comp==0) 
		{
			pthread_mutex_unlock(&map_lock);
			return p->val;
		}
		else if (comp<0) p=p->left;
		else p=p->right;
	}

	pthread_mutex_unlock(&map_lock);

	return MAP_NOT_FOUND;
}
/*
int main()
{
	int i;

	map_init();

	map_insert("nihao",1);
	map_insert("nishi",2);
	map_insert("nishi",3);
	map_delete("nishi");
	printf("%d %d\n",map_find("nishi"),map_find("nihao"));
	preorder(head);
	printf("\n");
	return 0;
}
*/
