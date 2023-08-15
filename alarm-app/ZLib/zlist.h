/*
*********************************************************************************************************
*	                                            UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : zlist.h
*    Module  : list
*    Version : V1.0
*    History :
*   -----------------
*             
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/

#ifndef __ZLIST_H__
#define __ZLIST_H__



/*
****************************************************************************************************
*                                             INCLUDES
****************************************************************************************************
*/

#ifndef NO_SYS
#include "uCOS_II.h"
#endif /*NO_SYS*/


#define LIST_DEFINE(LINST_NAME, LIST_STRUCT, LIST_SIZE) typedef struct LINST_NAME##list_n{\
	LIST_STRUCT    * p;\
	struct LINST_NAME##list_n  * next;\
}s##LINST_NAME##List_N;\
\
typedef struct {\
\
	  LIST_STRUCT     									buf[LIST_SIZE];\
																										 \
		s##LINST_NAME##List_N            	p[LIST_SIZE];  \
		s##LINST_NAME##List_N         *		rw_p;          \
		s##LINST_NAME##List_N  		    *		free_p;        \
} s##LINST_NAME##List;															 \
																							 

#define LIST_DEFINE_FUNC(LINST_NAME, LIST_STRUCT, LIST_SIZE) \
void LINST_NAME##_ListDataInit(s##LINST_NAME##List * listdata) \
{                                                                \
	int32_t i;                                                     \
	memset(listdata->buf, 0x00, sizeof(listdata->buf));            \
	                                                               \
	for(i = 0; i < LIST_SIZE -1; i++)                              \
	{                                                              \
		listdata->p[i].next = &(listdata->p[i + 1]);                 \
		listdata->p[i].p = &(listdata->buf[i]);                      \
	}                                                              \
	listdata->p[LIST_SIZE -1].next = NULL;                         \
	listdata->p[LIST_SIZE -1].p = &(listdata->buf[LIST_SIZE -1]);  \
	                                                               \
	listdata->free_p = listdata->p;                                \
	listdata->rw_p = NULL;                                         \
	                                                               \
}                                                                \
                                                                 \
int32_t LINST_NAME##_List_Write_data(s##LINST_NAME##List * list, LIST_STRUCT data) \
{                                                                                  \
	int err = 0;                                                                     \
	s##LINST_NAME##List_N *midpoint = NULL;                                                        \
	                                                                                 \
	if(list->free_p != NULL)                                                         \
	{                                                                                \
		midpoint = list->free_p;                                                       \
		list->free_p = list->free_p->next;                                             \
		*(midpoint->p) = data;                                                         \
		midpoint->next = list->rw_p;                                                   \
		list->rw_p = midpoint;                                                         \
	}                                                                                \
	else                                                                             \
		err = -1;                                                                      \
	return err;                                                                      \
}                                                                                  \
\
void LINST_NAME##_Delete_List(s##LINST_NAME##List * list,  \
	            s##LINST_NAME##List_N * before, s##LINST_NAME##List_N  * del)\
{\
	if(before == NULL)\
	{\
		list->rw_p = del->next; \
		del->next = list->free_p; \
		list->free_p = del;      \
	}\
	else\
	{\
		before->next = del->next;\
		del->next = list->free_p;\
		list->free_p = del; \
	}\
}\



	

#endif  /* __ZLIST_H__ */

