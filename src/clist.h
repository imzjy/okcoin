/* 
	single link list 
	imjatsz@gmail.com
*/

#ifndef __CLIST_H__
#define __CLIST_H__

typedef struct listNode{
	void 	*data;
	struct listNode 	*next;
} ListNode;

typedef struct list{
	struct listNode 	*head;
	struct listNode 	*tail;
	int size;
} List;


List *list_create();
void list_cleanup();

void list_append(List *l, void *data);


#endif /* __CLIST_H__ */