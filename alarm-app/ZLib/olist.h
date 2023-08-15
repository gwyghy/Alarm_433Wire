/*
*********************************************************************************************************
*	                                            UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : olist.h
*    Module  : can driver
*    Version : V1.0
*    History :
*   -----------------
*             
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/

#ifndef __OLIST_H__
#define __OLIST_H__











/*
****************************************************************************************************
*                                             INCLUDES
****************************************************************************************************
*/

#include "l4can.h"                                  /* CAN driver abstraction                   */

#ifndef NO_SYS
#include "uCOS_II.h"
#endif /*NO_SYS*/

#define UWB_APP_BUFF    4
typedef struct {
	uint16_t    tagId;
	uint16_t    time;
	uint16_t    heartTime;
	uint16_t    oldVal;
	uint16_t    val;
	uint16_t    state;
	uint16_t    wr;
	uint16_t    size;
	uint16_t    count;
	uint16_t    dist[UWB_APP_BUFF];
	uint8_t     right;
}sTagData;


#define LIST_SIZE    10

#define LIST_STRUCT  sTagData





/*------------------------------------------------------------------------------------------------*/
/*!
* \brief                      list BUFFER OBJECT
* \ingroup  
*
*          
*/
/*------------------------------------------------------------------------------------------------*/

//typedef struct list_o{
//	sTagData    * p;
//	struct list_o  * next;
//}zList_N;

//typedef struct {

//	  sTagData     		buf[LIST_SIZE];
//		
//		zList_N         pbuf[LIST_SIZE];
//		zList_N         *		rw_p;
//		zList_N  		    *		free_p;



//} zOList;


typedef struct list_n{
	LIST_STRUCT    * p;
	struct list_n  * next;
}sList_N;

typedef struct {
	LIST_STRUCT     	buf[LIST_SIZE];
		
	sList_N            	p[LIST_SIZE];
	sList_N        		*rw_p;
	sList_N  		   	*free_p;

} sOList;

/*
****************************************************************************************************
*                                     GLOBAL VARIABLE
****************************************************************************************************
*/


/*
****************************************************************************************************
*                                       FUNCTION PROTOTYPES
****************************************************************************************************
*/
void oListDataInit(sOList * listdata);
int32_t oList_Write_data(sOList * list, LIST_STRUCT data);


#endif  /* __OLIST_H__ */

