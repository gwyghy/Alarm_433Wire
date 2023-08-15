 /*
*********************************************************************************************************
*	                                           UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : olist.c
*    Module  : can driver
*    Version : V1.0
*    History :
*   -----------------
*             list data
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/
#include "olist.h"

#include <string.h>



/******************************************************************************
** 函数名称: oListDataInit(sOList * listdata)
** 功能描述: 初始化设备所需的数据结构
** 参数描述：
*******************************************************************************/
void oListDataInit(sOList * listdata)
{
	int32_t i; 
	memset(listdata->buf, 0x00, sizeof(listdata->buf));
	
	for(i = 0; i < LIST_SIZE - 1; i++)
	{
		listdata->p[i].next = &(listdata->p[i + 1]);
		listdata->p[i].p = &(listdata->buf[i]);
	}
	listdata->p[LIST_SIZE -1].next = NULL;
	listdata->p[LIST_SIZE -1].p = &(listdata->buf[LIST_SIZE -1]);
	
	listdata->free_p = listdata->p;
	listdata->rw_p = NULL;

}


int32_t oList_Write_data(sOList * list, LIST_STRUCT data)
{
	int err = 0;
	sList_N *midpoint = NULL;
	
	if(list->free_p != NULL)
	{
		midpoint = list->free_p;
		list->free_p = list->free_p->next;
		*(midpoint->p) = data;
		midpoint->next = list->rw_p;
		list->rw_p = midpoint;
	}
	else
		err = -1;

	return err;
}

