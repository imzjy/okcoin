#include <stdlib.h>
#include "clist.h"



List *list_create()
{
	List *l = (List *)malloc(sizeof(List));
	l->head = NULL;
	l->tail = NULL;
	l->size = 0;

	return l;
}

void list_append(List *l, void *data)
{
	ListNode *node = (ListNode *)malloc(sizeof(ListNode));
	node->next = NULL;
	node->data = data;

	if(!l->head){   //first element of array
		l->head = node;
		l->tail = node;
	} else {
		l->tail->next = node;
		l->tail = node;
	}

	l->size++;
}

void list_cleanup(List *l)
{
	if(l->size == 0)
		return;

	ListNode *cur = (ListNode *)l->head;
	while(cur){
		ListNode n = *cur;

		free(cur);
		cur = n.next;
	}
}