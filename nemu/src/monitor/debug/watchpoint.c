#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp()
{
	WP *f,*h;
	f = free_;
	free_=free_->next;
	f->next=NULL;
	h=head;
	if (h==NULL)
	{
		head=f;
		h=head;
	}
	else 
	{
		while (h->next!=NULL)
			h=h->next;
		h->next=f;
	}
	return f;
}

void free_wp (WP *wp)
{
	WP *f,*p;
	p = free_;
	if(p==NULL)
	{
		free_=wp;
		p=free_;
	}	
	else
	{
		while(p->next!=NULL)
			p=p->next;
		p->next=wp;
	}
	f=head;
	if(head==NULL) assert(0);
	if( head->NO == wp->NO)
		head = head->next;
	else
	{
		while(f->next != NULL&&f->next->NO!=wp->NO)
			f =f->next;
		if(f->next ==NULL&&f->NO==wp->NO)
			printf("Wrong\n");
		else if(f->next->NO==wp->NO)
			f->next=f->next->next;
		else 
			assert(0);
	}
	wp->next=NULL;
	wp->value=0;
	wp->b=0;
	wp->expr[0]='\0';
}

bool check_wp()
{
	WP *f;
	f = head;
	bool k=true;
	bool success;
	while(f != NULL)
	{
		uint32_t expr1=expr(f->expr,&success);
		if(!success)
			assert(1);
		if(expr1 != f->value)
		{
			k=false;
			if(f->b)
			{
				printf("Breakpoint %d at 0x%08x\n",f->b,cpu.eip);
				f = f->next;
				continue;
			}
			printf ("Watchpoint %d: %s\n",f->NO,f->expr);
			printf ("New value = %d\n",expr1);
			f->value = expr1;
		}
	}
	return k;
}







