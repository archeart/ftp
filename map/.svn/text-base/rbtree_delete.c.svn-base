#include <stdio.h>
#include <stdlib.h>
#include "struct.h"
#include "utils.h"

#define LL 0
#define RR 1

#define MAX_DEEP 20

static bool is_black(struct rbt_struct *root)
{
	if (!root||root->type==B) return 1;
	else return 0;
}

void rbt_delete_item(struct rbt_struct **head_ref,char *key)
{
	static struct rbt_struct **ref[MAX_DEEP];
	static bool child[MAX_DEEP];
	int ref_c=-1;
	struct rbt_struct *p=*head_ref;

	ref[++ref_c]=head_ref;

	while (p&&simple_strcmp(p->key,key)==0)
	{
		if (simple_strcmp2(key,p->key)==-1)
		{
			ref[++ref_c]=&p->left;
			child[ref_c]=LL;

			p=p->left;
		}
		else
		{
			ref[++ref_c]=&p->right;
			child[ref_c]=RR;

			p=p->right;
		}
	}

	if (p)
	{
		struct rbt_struct *u,*g,*s,*l,*r;
		struct rbt_struct **v;
		bool c;

		if (p->right)
		{
			u=p->right;

			ref[++ref_c]=&p->right;
			child[ref_c]=RR;

			for (u=p->right;u->left!=0;u=u->left)
			{
				ref[++ref_c]=&u->left;
				child[ref_c]=LL;
			}

			//p->val=u->val;
			/* copy start */
			p->val=u->val;

			free(p->key);
			p->key=u->key;
			/* copy end */

			v=ref[ref_c];
			c=child[ref_c];
			ref_c--;

			*v=u->right;

			if (ref_c==-1||u->type==R)
			{
				free(u);
				return;
			}
			/**/
			else
			{
				p=*ref[ref_c];
				free(u);
			}
		}
		else
		{
			u=p;

			v=ref[ref_c];
			c=child[ref_c];
			ref_c--;

			*v=u->right;
			

			/*top head deleted or no height violation*/
			if (ref_c==-1||u->type==R)
			{
				free(u);
				return;
			}
			else
			{
				p=*ref[ref_c];
				free(u);
			}
		}
		
		while (ref_c>=0)
		{
			if (c==LL)
			{
				s=p->right;
				l=s->left;
				r=s->right;
				
				if (s->type==R)
				{
					p->type=R;
					s->type=B;

					p->right=l;
					s->left=p;

					v=ref[ref_c];
					*v=s;

					ref[++ref_c]=&s->left;

					s=p->right;
					l=s->left;
					r=s->right;
				}

				if (!is_black(l))
				{
					s->left=l->right;
					p->right=l->left;

					l->type=p->type;
					l->left=p;
					l->right=s;
					p->type=B;

					v=ref[ref_c];
					*v=l;
					return;
				}
				else if (p->type==R)
				{
					p->type=R;
					s->type=B;

					p->right=l;
					s->left=p;

					v=ref[ref_c];
					*v=s;
					return;
				}
				else if (!is_black(r))
				{
					p->right=l;
					s->left=p;

					r->type=B;

					v=ref[ref_c];
					*v=s;
					return;
				}
				else
				{
					s->type=R;

					c=child[ref_c--];

					if (ref_c>=0) p=*ref[ref_c];
				}
			}
			else
			{
				s=p->left;
				l=s->left;
				r=s->right;
				
				if (s->type==R)
				{
					p->type=R;
					s->type=B;

					p->left=r;
					s->right=p;

					v=ref[ref_c];
					*v=s;

					ref[++ref_c]=&s->right;

					s=p->left;
					l=s->left;
					r=s->right;
				}

				if (!is_black(r))
				{
					s->right=r->left;
					p->left=r->right;

					r->type=p->type;
					r->right=p;
					r->left=s;
					p->type=B;

					v=ref[ref_c];
					*v=r;
					return;
				}
				else if (p->type==R)
				{
					p->type=R;
					s->type=B;

					p->left=r;
					s->right=p;

					v=ref[ref_c];
					*v=s;
					return;
				}
				else if (!is_black(l))
				{
					p->left=r;
					s->right=p;

					l->type=B;

					v=ref[ref_c];
					*v=s;
					return;
				}
				else
				{
					s->type=R;

					c=child[ref_c--];

					if (ref_c>=0) p=*ref[ref_c];
				}
			}
		}
	}
}
