/*
 * Quick'n'dirty type-specific linked list code
 * Copyright (c) 2008 Arthur Huillet
 */

#include "struct.h"
#define _LISTS_C

#include "proto.h"

#define add_obj_to_list_head(TYPE) { \
    TYPE * newobj = malloc(sizeof(TYPE));\
    memcpy(newobj, toadd, sizeof(TYPE));\
    newobj->NEXT = head;\
    newobj->PREV = NULL;\
    if(head)\
            head->PREV = newobj;\
    return newobj;\
    }

#define rem_obj_from_list() {\
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

enemy * add_enemy_head(enemy * head, enemy * toadd)
{
    add_obj_to_list_head(enemy)
}

enemy * del_enemy(enemy * head, enemy * todelete)
{
    rem_obj_from_list()
}

int free_enemy_list(enemy * head)
{
    free_obj_list1(enemy);
    free_obj_list2(enemy);
}

int move_enemy(enemy ** newhead, enemy * src, enemy ** srchead)
{
	if ( src == *srchead ) 
	    { /*we're moving the head of the source list*/
	    (*srchead) = src->NEXT;
	    }

	if ( * newhead )
	    (*newhead)->PREV = src;

	if ( src->PREV )
	    src->PREV->NEXT = src->NEXT;

	if ( src->NEXT )
	    src->NEXT->PREV = src->PREV;

	src->PREV = NULL;
	src->NEXT = (*newhead);
	(*newhead) = src;

	return 0;
}


#undef _LISTS_C

