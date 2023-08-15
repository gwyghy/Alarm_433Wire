/*
*********************************************************************************************************
*	                                            UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : location_data_app.h
*    Module  : 
*    Version : V1.0
*    History :
*   -----------------
*             
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/

#ifndef __LOCATION_DATA_APP_H__
#define __LOCATION_DATA_APP_H__

#include "olist.h"



#define TAG_OFF_TIME     5000
#define TAG_COUNT_CYCLE  250
#define TAG_OFF_COUNT    (TAG_OFF_TIME/TAG_COUNT_CYCLE)

#define CONTROL_SEND_TASK              0

enum {
	TAG_ENTER = 1,
	TAG_OFF,
	TAG_ERROR
};


enum {
	TAG_DEV_OK = 0,
	FIRST_OFF_ERR = 1,
	TAG_DEV_OFF_LINE,
	
};


typedef struct
{
	uint8_t  tagNum;   //测量范围的标签数量
}sUwbContInfo;

#define REPORT_D_DISTANCE  5

#define REPORT_TIME_INTERVAL  40


/****************************************************************************************************
 CAN rx and tx task  stacks
 ***************************************************************************************************/
#define		UWBAPP_TASK_SIZE                        256u
#define		TAG_REPORT_TASK_SIZE                    256u

void LocationDataAppInit(void);


uint8_t GetPeopleNum(void);
void EnterPeopleAdd(void);
void ExitPeoplePlus(void);
int DeleteSecondListData(uint16_t  tagId);

#endif /*__LOCATION_DATA_APP_H__*/


