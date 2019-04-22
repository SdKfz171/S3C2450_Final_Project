/*****************************************************************************
*                                                                            *
*  -------------------------------- list.h --------------------------------  *
*                                                                            *
*****************************************************************************/

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

/*****************************************************************************
*                                                                            *
*  Define a structure for linked list elements.                              *
*                                                                            *
*****************************************************************************/

typedef struct ListElmt_ {

char              data[512];
struct ListElmt_   *next; //자기 참조 구조체 

} ListElmt;

/*****************************************************************************
*                                                                            *
*  Define a structure for linked lists.                                      *
*                                                                            *
*****************************************************************************/

typedef struct List_ {

int                size;

int                (*match)(const void *key1, const void *key2);
void               (*destroy)(void *data);

ListElmt           *head;	//중첩 구조체 
ListElmt           *tail;

} List;


/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

void list_init(List *list, void (*destroy)(void *data));

void list_destroy(List *list);

int list_ins_next(List *list, ListElmt *element, char  *data);

int list_rem_next(List *list, ListElmt *element, void **data);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define list_size(list) ((list)->size)									/////////////////////////////////////
																		/////////  매크로  ////////////////// 
#define list_head(list) ((list)->head)									/////////////////////////////////////
																		/////////////////////////////////////
#define list_tail(list) ((list)->tail)

#define list_is_head(list, element) ((element) == (list)->head ? 1 : 0)

#define list_is_tail(element) ((element)->next == NULL ? 1 : 0)

#define list_data(element) ((element)->data)

#define list_next(element) ((element)->next)

#endif
