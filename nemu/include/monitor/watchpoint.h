#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	bool in_use;
	uint32_t val;
	char *expr;
	/* TODO: Add more members if necessary */


} WP;

int set_watchpoint(char *args);
void list_watchpoint();
int hit_watchpoint();
bool del_watchpoint(int num);
void del_all_watchpoint();

#endif
