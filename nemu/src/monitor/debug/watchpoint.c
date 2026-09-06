#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include <stdlib.h>

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
		wp_pool[i].in_use=false;
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
	assert(free_!=NULL);
	WP *p=free_;
	free_=free_->next;
	assert(p->in_use==false);
	p->in_use=true;
	return p;
}

void free_wp(WP *wp){
	assert(wp!=NULL);
	wp->in_use=false;
	wp->next=free_;
	free_=wp;
}

int set_watchpoint(char *args){
	bool success=true;
	uint32_t addr=expr(args,&success);
	if(success){
		WP *p=new_wp();
		p->expr=strdup(args);
		p->val=addr;
		p->next=head;
		head=p;
		return p->NO;
	}
	else return -1;
}

void del_watchpoint(int num){
	assert(wp_pool[num].in_use==true);
	WP *wp=&wp_pool[num];
	WP *prev=NULL,*cur=head;
	while(cur != NULL && cur != wp){
        prev = cur;
        cur = cur->next;
    }
	assert(cur==wp);
	wp->in_use=false;
	if(wp==head){
		head=head->next;
	}
	else if(wp->next!=NULL){
		prev->next=wp->next;
	}
	else{
		prev->next=NULL;
	}
	free_wp(wp);
}

void list_watchpoint(){
	if(head==NULL){
		printf("No watchpoints\n");
		return;
	}
	else{
		printf("%-8s","Num");
		printf("%-16s","Address");
		printf("%-24s","Expression");
		printf("\n");
		WP *cur=head;
		while(cur!=NULL){
			printf("%-8d",cur->NO);
			printf("0x%-14.8x",cur->val);
			printf("%-24s",cur->expr);
			printf("\n");
			cur=cur->next;
		}
	}
}

int hit_watchpoint(){
	WP *cur=head;
	while(cur!=NULL){
		bool success=true;
		char *p=strdup(cur->expr);
		uint32_t position=expr(p,&success);
		// printf("%08x\t%08x\n",position,cur->val);
		free(p);
		p=NULL;
		if(position!=cur->val){
			return cur->NO;
		}
		cur=cur->next;
	}
	return -1;
}