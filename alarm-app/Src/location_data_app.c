/*
*********************************************************************************************************
*	                                            UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : location_data_app.c
*    Module  : uwb
*    Version : V1.0
*    History :
*   -----------------
*              uwb app 
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/

#include "location_data_app.h"
#include "can_app.h"
#include "main.h"
#include "uwb_anchor.h"

#include "app_cfg.h"
/********************************************************************************
*变量定义
*********************************************************************************/

OS_STK UWB_APPstk[UWBAPP_TASK_SIZE];
#if CONTROL_SEND_TASK == 1	
OS_STK TAG_Reportstk[TAG_REPORT_TASK_SIZE];
OS_EVENT		* gTagDataSem;	
#endif /*CONTROL_SEND_TASK == 1*/

sOList gUwbListData;

sUwbContInfo gUwbConInfo;

sList_N * gTimeReport = NULL;

/*******************************************************************************************
**函数名称：
**输　入：None
**输　出：None
**功能描述：监测范围内标签个数相关操作
*******************************************************************************************/
__inline void EnterPeopleAdd(void)
{
	gUwbConInfo.tagNum ++;    
}

__inline void ExitPeoplePlus(void)
{
	gUwbConInfo.tagNum > 0 ? gUwbConInfo.tagNum-- : 0;
}

__inline uint8_t GetPeopleNum(void)
{
	return gUwbConInfo.tagNum;
}

/*******************************************************************************************
**函数名称：int32_t CheckIsReport(sTagData * data)
**输　入：None
**输　出：0:不需要上报 1：上报
**功能描述：检测是否上报数据
*******************************************************************************************/
int32_t CheckIsReport(sTagData * data)
{
	uint16_t dvalue = (data->val > data->oldVal) ? (data->val - data->oldVal) : (data->oldVal - data->val);

	if(dvalue > REPORT_D_DISTANCE)
	{
		data->oldVal = data->val;
		return 1;
	}
	
	return 0;
}

/*******************************************************************************************
**函数名称：void HeartReportProcess(sTagData * data)
**输　入：None
**输　出：无
**功能描述：上报定位数据信息
*******************************************************************************************/
void HeartReportProcess(sTagData * data)
{
	data->heartTime += TAG_COUNT_CYCLE;
	if(data->heartTime >= GetUwbReportTime())
	{
		WriteReportInfo(GetPeopleNum(), data->tagId, data->val, TAG_ENTER, data->right);// 写发送数据
//		OSSemPost(gTagDataSem);
		data->heartTime = 0;
	}
}

/*******************************************************************************************
**函数名称：void NewTagData(uint16_t tagId, uint16_t dist, sTagData * data)
** 输　入：None
** 输　出：None
** 功能描述：Can发送任务
*******************************************************************************************/
void NewTagData(uint16_t tagId, uint16_t dist, uint8_t right, sTagData *data)
{
	memset(data, 0x00, sizeof(sTagData));
	data->tagId = tagId;
	data->time = TAG_OFF_TIME;
	data->wr = 1;
	data->size = 1;
	data->dist[0] = dist;
	data->val = dist;
	data->oldVal = dist;
	data->state = TAG_ENTER;
	data->right = right;
}

/*******************************************************************************************
**函数名称：void TagDataProcess(sTagData * data, uint16_t dist)
**输　入：None
**输　出：None
**功能描述：标签数据处理计算
*******************************************************************************************/
void TagDataProcess(sTagData * data, uint16_t dist, uint8_t right)
{
	uint16_t num = 0;
	uint16_t i;
	uint32_t tmp = 0;
	
	if(data->size < UWB_APP_BUFF)
	{
		data->dist[data->size++] = dist;
		data->wr++;
		data->wr %= UWB_APP_BUFF;
		num = data->size;
	}
	else
	{
		data->dist[data->wr++] = dist;
		data->wr %= UWB_APP_BUFF;
		num = UWB_APP_BUFF;
	}
	for(i = 0; i < num; i++)
	{
		tmp += data->dist[i];
	}
	data->time = TAG_OFF_TIME;
	data->val = tmp / num;
	data->right = right;
}

/*******************************************************************************************
**函数名称：void FirstAddProcess(uint16_t tagId, uint16_t dist)
**输　入：None
**输　出：None
**功能描述：第一次添加标签处理函数
*******************************************************************************************/
int FirstAddProcess(uint16_t tagId, uint16_t dist, uint8_t right)
{
	sTagData tmp;

	if(dist > gUwbRunPara.extent)					//超过检测范围的tag
	{
		return FIRST_OFF_ERR;//标签第一次出现即离开，不做处理
	}
	else
	{
		EnterPeopleAdd();                    //检测范围内标签个数增加
		NewTagData(tagId, dist, right, &tmp);
		oList_Write_data(&gUwbListData, tmp);
		WriteReportInfo(GetPeopleNum(), tagId, dist, TAG_ENTER, right);  //写发送的can数据
#if CONTROL_SEND_TASK == 1
		OSSemPost(gTagDataSem);
#endif /*CONTROL_SEND_TASK == 1*/
		return TAG_DEV_OK;
	}
}
/*******************************************************************************************
**函数名称：void TagExitProcess(uint16_t tagId, uint16_t dist)
**输　入：None
**输　出：None
**功能描述：标签离开监测范围处理函数
*******************************************************************************************/
void TagExitProcess(uint16_t tagId, uint16_t dist, uint8_t right)
{
	ExitPeoplePlus();
	WriteReportInfo(GetPeopleNum(), tagId, dist, TAG_OFF, right);       //写发送的can数据
#if CONTROL_SEND_TASK == 1
	OSSemPost(gTagDataSem);
#endif /*CONTROL_SEND_TASK == 1*/
}

/*******************************************************************************************
**函数名称：void TagOffLineProcess(uint16_t tagId, uint16_t dist)
**输　入：None
**输　出：None
**功能描述：标签掉线处理函数
*******************************************************************************************/
void TagOffLineProcess(uint16_t tagId, uint16_t dist, uint8_t right)
{
	ExitPeoplePlus();
	WriteReportInfo(GetPeopleNum(), tagId, dist, TAG_ERROR, right);       //写发送的can数据
#if CONTROL_SEND_TASK == 1
	OSSemPost(gTagDataSem);
#endif /*CONTROL_SEND_TASK == 1*/
}

/*******************************************************************************************
**函数名称：void TagOffLineProcess(uint16_t tagId, uint16_t dist)
**输　入：None
**输　出：None
**功能描述：标签掉线处理函数
*******************************************************************************************/
//int GetTimeReportData(void)
//{
//		if(gTimeReport != NULL)   //有标签能够发送
//		{
//			WriteReportInfo(GetPeopleNum(), gTimeReport->p->tagId, gTimeReport->p->val, TAG_ENTER);// 写发送数据
//			gTimeReport = gTimeReport->next;
//			
//		}
//	
//}
/*******************************************************************************************
**函数名称：
**输　入：None
**输　出：None
**功能描述：删除二级链表
*******************************************************************************************/
int DeleteSecondListData(uint16_t tagId)
{
	int err = 0;
	sList_N * tempoint = NULL;
	sList_N * retpoint = NULL;
	sList_N * midpoint = gUwbListData.rw_p;
//	tagId = tag_p->tagId;
//	dist = tag_p->realDist;
	
//	OS_ENTER_CRITICAL();
	if(midpoint == NULL)   //空buf
	{
		err = 1;
	}
	
	while(midpoint != NULL) //遍历
	{
		if(midpoint->p->tagId == tagId)  //存在tagId
		{
			tempoint = midpoint;
			break;
		}
		retpoint = midpoint;
		midpoint = midpoint->next;
	}
	
	if(tempoint != NULL)  //存在tagId
	{
		TagOffLineProcess(tagId, tempoint->p->val, tempoint->p->right);
		/******删除链表操作**************/
		if(retpoint == NULL)
		{
			gUwbListData.rw_p = tempoint->next;
			tempoint->next = gUwbListData.free_p;
			gUwbListData.free_p = tempoint;
		}
		else
		{
			retpoint->next = tempoint->next;
			tempoint->next = gUwbListData.free_p;
			gUwbListData.free_p = tempoint;
		}
	}
	else 
	{
		err = 1;
	}
	
	return err;
}

/*******************************************************************************************
**函数名称：int WriteTagData(uint16_t tagId, uint16_t dist)
**输　入：None
**输　出：None
**功能描述：将数据放入二级缓存中
*******************************************************************************************/
void ReInitDataBuf(sDistNote * tag_p)
{
	memset(tag_p, 0x00, sizeof(sDistNote));
}

int CheckTagOffLine(sDistNote * tag_p)
{
	if(tag_p->oldHeartCout == tag_p->heartCout)
	{
		if(tag_p->errCount >= TAG_OFF_COUNT)
			return TAG_DEV_OFF_LINE;
		else
			tag_p->errCount++;
	}
	else
	{
		tag_p->oldHeartCout = tag_p->heartCout;
		tag_p->errCount = 0;
		
	}
	return TAG_DEV_OK;
}


int WriteTagData(sDistNote * tag_p)
{
	uint16_t tagId;
	uint16_t dist;

//#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
//    	OS_CPU_SR  cpu_sr = 0;
//#endif
//	sTagData tmp;
//	uint8_t err = 0;
	int err = 0;
	sList_N * tempoint = NULL;
	sList_N * retpoint = NULL;
	sList_N * midpoint = gUwbListData.rw_p;
	tagId = tag_p->tagId;
	dist = tag_p->realDist;
	
//	OS_ENTER_CRITICAL();
	if(midpoint == NULL)   //空buf
	{
		err = FirstAddProcess(tagId, dist, tag_p->right);
		goto WEnd;
	}
	
	while(midpoint != NULL) //遍历
	{
		if(midpoint->p->tagId == tagId)  //存在tagId
		{
			tempoint = midpoint;
			break;
		}
		retpoint = midpoint;
		midpoint = midpoint->next;
	}
	
	if(tempoint != NULL)  //存在tagId
	{
		err = CheckTagOffLine(tag_p);    //设备离线错误判断
		if(err == TAG_DEV_OFF_LINE)
		{
			TagOffLineProcess(tagId, dist, tag_p->right);
		}
		else
		{
			TagDataProcess(tempoint->p, dist, tag_p->right);  //新数据处理
			err = CheckTagState(tempoint->p->val);
			if(err == 1)  //离开监测范围
			{
				TagExitProcess(tagId, tempoint->p->val, tempoint->p->right);
			}
		}
		if(err)  
		{
			/******删除链表操作**************/
			if(retpoint == NULL)
			{
				gUwbListData.rw_p = tempoint->next;
				tempoint->next = gUwbListData.free_p;
				gUwbListData.free_p = tempoint;
			}
			else
			{
				retpoint->next = tempoint->next;
				tempoint->next = gUwbListData.free_p;
				gUwbListData.free_p = tempoint;
			}
		}
		else
		{
			//在监测范围内查看是否满足变化距离，满足则上报
			if(CheckIsReport(tempoint->p))
			{
				tempoint->p->heartTime = 0;
//				if(tag_p->commCnt > 4)
					WriteReportInfo(GetPeopleNum(), tagId, tempoint->p->val, TAG_ENTER, tempoint->p->right);   //写发送的can数据
#if CONTROL_SEND_TASK == 1
				OSSemPost(gTagDataSem);
#endif /*CONTROL_SEND_TASK == 1*/
			}
#if CONTROL_SEND_TASK == 0
			else
			{
				HeartReportProcess(tempoint->p);
			}
#endif /*CONTROL_SEND_TASK == 0*/
		}
	}
	else 
	{
		err = FirstAddProcess(tagId, dist, tag_p->right);
	}
	
WEnd:	
//	OS_EXIT_CRITICAL();
	
//	if(err != TAG_DEV_OK)
//	{
//		ReInitDataBuf(tag_p);   //删除一级链表中的数据
//	}
	return err;
}


/*******************************************************************************************
**函数名称：void PollCheckTagOffLine(void)
**输　入：None
**输　出：None
**功能描述：循环检查标签是否离线
*******************************************************************************************/
void PollCheckTagOffLine(void)
{
	sList_N * tempoint = NULL;
	sList_N * retpoint = NULL;
	sList_N * midpoint = gUwbListData.rw_p;
	
	if(midpoint == NULL)   //空buf
	{
		return;
	}
	
	while(midpoint != NULL) //遍历
	{
		if(midpoint->p->time != TAG_OFF_TIME)  //未更新
		{
			tempoint = midpoint->next;
//			ExitPeoplePlus();
			TagOffLineProcess(midpoint->p->tagId, midpoint->p->val, midpoint->p->right);
			if(retpoint == NULL)
			{
				gUwbListData.rw_p = midpoint->next;
				midpoint->next = gUwbListData.free_p;
				gUwbListData.free_p = midpoint;
			}
			else
			{
				retpoint->next = midpoint->next;
				midpoint->next = gUwbListData.free_p;
				gUwbListData.free_p = midpoint;
			}
			midpoint = tempoint;
		}
		else
		{
			midpoint->p->time = 0;
			retpoint = midpoint;
			midpoint = midpoint->next;
		}
	}
}

/*******************************************************************************************
**函数名称：void UpdateDataProcess(void)
**输　入：None
**输　出：None
**功能描述：处理一级跟二级buff数据
*******************************************************************************************/
void UpdateDataProcess(void)
{
//	uint8_t i;
	#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register */
    	OS_CPU_SR  cpu_sr = 0;
	#endif
	sDistList_N * midPoint;
	sDistList_N * beforeP;
	sDistList_N * deletP;
	OS_ENTER_CRITICAL();
	
	for(midPoint = GetDistBufPoint(), beforeP = NULL; midPoint != NULL; )
	                               //   beforeP = midPoint, midPoint = midPoint->next)
	{
		DistBufDataCount(midPoint->p);
//		if((midPoint->p->commCnt > 3) || (midPoint->p->dist[midPoint->p->commCnt - 1] > gUwbRunPara.extent))
		if((midPoint->p->commCnt > 3) || ((midPoint->p->commCnt > 0) && (midPoint->p->dist[midPoint->p->commCnt - 1] > gUwbRunPara.extent)))	//通讯次数有效或超过检测距离，并且超过检测距离的有通讯次数
		{
			if(WriteTagData(midPoint->p) != TAG_DEV_OK)
			{
				deletP = midPoint;
				midPoint = midPoint->next;
				DelDistBufData(beforeP, deletP);
			}
			else
			{
				beforeP = midPoint;
				midPoint = midPoint->next;
			}
		}
		else
		{
			beforeP = midPoint;
			midPoint = midPoint->next;
		}
	}
//	for(i = 0; i < ANC_COMM_NUM_MAX_WITH_TAG; i++)
//	{
//		if(TagDistBuff[i].TagID != 0 && TagDistBuff[i].mark == 1)    //存在数据进行处理
//		{
//			WriteTagData(TagDistBuff+i);
//		}
//	}
	OS_EXIT_CRITICAL();
}

/*******************************************************************************************
**函数名称：void UwbAppTask(void *p_arg)
**输　入：None
**输　出：None
**功能描述：UWB data task
*******************************************************************************************/
void UwbAppTask(void *p_arg)
{
	while(1)
	{
		UpdateDataProcess();
//		PollCheckTagOffLine();
		OSTimeDly(TAG_COUNT_CYCLE);			//250ms
	}
}

/*******************************************************************************************
**函数名称：void TagReportTask(void *p_arg)
**输　入：None
**输　出：None
**功能描述：
*******************************************************************************************/
#if CONTROL_SEND_TASK == 1
	void TagReportTask(void *p_arg)
	{
		INT8U err;

		while(1)
		{
			gTimeReport = gUwbListData.rw_p;
			OSSemPend(gTagDataSem, GetUwbReportTime(), &err);
			if( err == OS_ERR_NONE)
			{
				//gTimeReport = gUwbListData.rw_p;
				;
			}
			else if(err == OS_ERR_TIMEOUT)
			{
				if(gTimeReport != NULL)   //有标签能够发送
				{
					WriteReportInfo(GetPeopleNum(), gTimeReport->p->tagId, gTimeReport->p->val, TAG_ENTER);// 写发送数据
					gTimeReport = gTimeReport->next;
					while(gTimeReport != NULL)
					{
					
						OSSemPend(gTagDataSem, REPORT_TIME_INTERVAL, &err);
						if(err == OS_ERR_TIMEOUT)
						{
							if(gTimeReport != NULL)   //有标签能够发送
							{
								WriteReportInfo(GetPeopleNum(), gTimeReport->p->tagId, gTimeReport->p->val, TAG_ENTER);// 写发送数据
								gTimeReport = gTimeReport->next;
								
							}
						}
						else
						{
							break;
						}
					}
				}
			}
			else
			{
				;//
			}
		}
	}
#endif /*CONTROL_SEND_TASK == 1*/


/******************************************************************************
** 函数名称: void LocationDataAppInit(void)
** 功能描述: 初始化UWB列表参数，创建UWB数据管理任务
** 参数描述：无
*******************************************************************************/
void LocationDataAppInit(void)
{
	INT8U err;

	/* data init */ 
#if CONTROL_SEND_TASK == 1
	gTagDataSem = OSSemCreate(0);
#endif /*CONTROL_SEND_TASK == 1*/
	oListDataInit(&gUwbListData);
#if CONTROL_SEND_TASK == 1	
//	err = OSTaskCreateExt((void (*)(void *))TagReportTask,
					(void          * )0,
					(OS_STK        * )&TAG_Reportstk[TAG_REPORT_TASK_SIZE - 1],
					(uint8_t         )TAG_REPORT_TAST_PRIO,
					(uint16_t        )TAG_REPORT_TAST_PRIO,
					(OS_STK        * )&TAG_Reportstk[0],
					(INT32U          )TAG_REPORT_TASK_SIZE,
					(void          * )0,
					(uint16_t        )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
	if (err == OS_ERR_NONE)
	{
		OSTaskNameSet(UWBAPP_TASK_PRIO, (uint8_t *)"TAG REPORT Task", &err);
	}
#endif /*CONTROL_SEND_TASK == 1*/	

	err = OSTaskCreateExt((void (*)(void *))UwbAppTask,
					(void          * )0,
					(OS_STK        * )&UWB_APPstk[UWBAPP_TASK_SIZE - 1],
					(uint8_t         )UWBAPP_TASK_PRIO,
					(uint16_t        )UWBAPP_TASK_PRIO,
					(OS_STK        * )&UWB_APPstk[0],
					(INT32U          )UWBAPP_TASK_SIZE,
					(void          * )0,
					(uint16_t        )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

	if (err == OS_ERR_NONE)
	{
		OSTaskNameSet(UWBAPP_TASK_PRIO, (uint8_t *)"UWB APP Task", &err);
	}
}

