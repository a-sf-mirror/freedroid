/*
 * Quick'n'dirty linked list code based on macros
 * Copyright (c) 2008 Arthur Huillet
 */

#include "struct.h"
#define _LISTS_C

#include "proto.h"

#define define_add_obj_to_list_head(TYPE) \
TYPE * add_##TYPE##_head(TYPE * head, TYPE * toadd)\
{ \
    TYPE * newobj = malloc(sizeof(TYPE));\
    memcpy(newobj, toadd, sizeof(TYPE));\
    newobj->NEXT = head;\
    newobj->PREV = NULL;\
    if(head)\
            head->PREV = newobj;\
    return newobj;\
    }

#define define_rem_obj_from_list(TYPE) \
TYPE * del_##TYPE(TYPE * head, TYPE * todelete)\
{\
    if(todelete == head)\
	{ /*if the object is the first of the list*/\
	head = GETNEXT(todelete);\
	if(head)\
	    head->PREV = NULL;\
	free(todelete);\
	return head;\
	}\
    else if(!GETNEXT(todelete))\
	{ /*if the object is the last of the list*/\
	if(GETPREV(todelete))\
	    GETPREV(todelete)->NEXT = NULL;\
	free(todelete);\
	return head;\
	}\
    if(todelete != head && GETNEXT(todelete))\
	{ \
	(GETPREV(todelete))->NEXT = GETNEXT(todelete);\
	(GETNEXT(todelete))->PREV = GETPREV(todelete);\
	free(todelete);\
	return head;\
	}\
    return NULL;\
    }

#define free_obj_list1(TYPE)  \
    TYPE * objrot = head;\
    TYPE * nextptr;\
    while(objrot)\
        {\
        nextptr = GETNEXT(objrot);

#define free_obj_list2(TYPE) \
	free(objrot);\
        objrot = nextptr;\
    }\
    return 0;

#define define_move_obj(TYPE)\
    int move_##TYPE(TYPE ** newhead, TYPE * src, TYPE ** srchead)\
{\
    if ( src == *srchead )\
	{ /*we're moving the head of the source list*/\
	(*srchead) = src->NEXT;\
	}\
\
    if ( * newhead )\
	(*newhead)->PREV = src;\
\
    if ( src->PREV )\
	src->PREV->NEXT = src->NEXT;\
\
    if ( src->NEXT )\
	src->NEXT->PREV = src->PREV;\
\
    src->PREV = NULL;\
    src->NEXT = (*newhead);\
    (*newhead) = src;\
\
    return 0;\
}


define_add_obj_to_list_head(enemy)
define_rem_obj_from_list(enemy)
define_move_obj(enemy)

int free_enemy_list(enemy * head)
{
    free_obj_list1(enemy);
    free_obj_list2(enemy);
}

#undef _LISTS_C

