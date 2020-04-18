#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"
#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
int init = 0;
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp()
{
	if(init == 0)
	{
		init_wp_pool();
	}
	WP *p = free_;
	if(p)
	{
		free_ = free_->next;
		p->next = head;
		head = p;
		init++;
		return p;
	}
	else
	{
		assert(0);
	}
}

int free_up(int NO)
{
	WP *p = head;
	WP *pre = p;
	if(head == NULL)
	{
		printf("No WatchPoint\n");
		return 0;
	}
	if(head->NO == NO)
	{
		head = head->next;
		p->next = free_;
		free_ = p;
		printf("WatchPoint %d has been deleted\n",p->NO);
		return 1;
	}
	else
	{
		while(p != NULL && p->NO != NO)
		{
			pre = p;
			p = p->next;
		}
		if(p->NO == NO)
		{
			pre->next = p->next;
			p->next = free_;
			free_ = p;
			printf("WatchPoint %d has been deleted\n",p->NO);
			init--;
			return 1;
		}
	}
	return 0;
}

int set_watchpoint(char *e)
{
	WP *p;
	p = new_wp();
	printf("Set Watchpoint #%d\n", p->NO);
	strcpy(head->expr,e);
	printf("expr = %s\n",p->expr);
	bool success = true;
	p->old_val = expr(p->expr,&success);
	if(!success)
	{
		printf("Fail to eval!\n");
		return 0;
	}
	else
	{
		printf("Old value = %#x\n",p->old_val);
	}
	return 1;
}

bool delete_watchpoint(int NO)
{
	if(free_up(NO)) return true;
	else 
	{
		printf("Watchpoing deleted failed!\n");
		return false;
	}
}

void list_watchpoint(void)
{
	WP *p = head;
	if(p == NULL)	printf("No WatchPoint!\n");
	else
	{
		while(p)
		{
			printf("%2d %-25s%#x\n",p->NO,p->expr,p->old_val);
			p = p->next;
		}
	}
}

WP* scan_watchpoint(void)
{
	WP *p = head;
	bool success = true;
	if(p == NULL)
	{
		printf("No Watchpoint\n");
		return NULL;
	}
	else
	{
		while(p)
		{
			p->new_val = expr(p->expr,&success);
			if(!success)	printf("Fail to eval new_val in WatchPoint %d\n",p->NO);
			else
			{
				if(p->new_val != p->old_val)
				{
					printf("Hit WatchPoint %d at address %#8x\n",p->NO,cpu.eip);
					printf("expr    =  %s\n",p->expr);
					printf("Old value = %#x\n",p->old_val);
					printf("New value = %#x\n",p->new_val);
					p->old_val = p->new_val;
					printf("program has been paused\n");
					return p;
				}
			}
		}
	}
	return NULL;
}





/* TODO: Implement the functionality of watchpoint */


