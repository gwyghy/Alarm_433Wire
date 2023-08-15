/**
  ******************************************************************************
  * File Name          : anchor.c
  * Description        : This file provides code for the configuration
  *                      of the anchor instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
//#include "uwb_app.h"
#define __UWB_ANCHOR_C__
#include "dw1000_bus.h"
#include "uwb_anchor.h"
#include "uwb_common.h"
#include "location_data_app.h"
#include "deca_device_api.h"
#include "uwb_debug.h"

//#include "zlist.h"

// ����һ����������
//LIST_DEFINE(Dist, sDistNote, ANC_LOC_TAG_MAX)

uint16_t tagIdTest = 431;

uint32_t gUwbAncState;
uint32_t gUwbAncErrState;
uint8_t gAncRxBuff[ANC_RX_LEN_MAX];

OS_EVENT			*gDwSendSem;
OS_EVENT			*gDwSendOkSem;

sAncDevInfo gAncDev;
sLocTime    gLocTime;
uint16_t    gDist;
uint8_t     gRight;   //�¼ӱ�ǩȨ����Ϣ
sDistList  gDistDataBuf;
sTagTime   gTagTime;

uint32_t   gAncErrorId;
uint16_t gTestId;
//uint32_t gTestErr;
extern uint32_t gUwbState;
//dwt_rxdiag_t gRxDiag;

INT32U        gTxTimeover;
INT32U        gUwbTimeOver;

static sDW100ConfigPara gDwAnchorConfig =
{
	ANCHOR_RX_ANT_DLY,
	ANCHOR_TX_ANT_DLY,
	ANCHOR_PAN_ID,
	0	
};

/*��վ*/
/*                            ֡����     ֡���к�  Ŀ��PANID      Ŀ���ַ    Դ��ַ     ֡������  
                               2�ֽ�       1�ֽ�       2�ֽ�        2�ֽ�       2�ֽ�       1�ֽ�  */
uint8_t gAnc_CommDecResp[ANC_COMM_ACK_LEN] = {0x41, 0x88, 0, 0xCA, 0xDE, 0xFF, 0xFF, 'V', 'E', FRAME_COMM_DEC_RESP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};		//��λͨѶ����֡�ظ�
//uint8_t gRxPollMsg[] = {0x41, 0x88,    0,    0xCA, 0xDE,   0xFF, 0xFF,   'V', 'E', FRAME_POLL, 0, 0};													//POLL֡
uint8_t gTxRespMsg[ANC_TX_FRAME_LEN]  = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 0xFF, 0xFF, FRAME_RESP, 0x02, 0, 0, 0};								//RESP֡
//uint8_t gRxFinalMsg[] = {0x41, 0x88,    0,    0xCA, 0xDE,   0xFF, 0xFF,    'V', 'E', FRAME_FINAL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};			//FINAL֡


void SetAnchorAddr(uint8_t * dest, uint16_t addr)
{
	memcpy(dest + ANCHOR_ADDR_OFF, (void *)&addr, 2);
}

void DebugErrorProcess(void)
{
	while (1)
	{
		;
	}
}


__inline void RestartRxProcess(void)
{
	DWT_SetRxTimeOut(0);								//�趨���ճ�ʱʱ�䣬0--û�г�ʱʱ�䣬���޵ȴ�
	DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//�򿪽���;
}

void AncErrorProcess(uint32_t err)
{
	gAncErrorId = err;
	RestartRxProcess();
//	while (1)
//	{
//		;
//	}
}

/*******************************************************************************************
* �������ƣ�bool caculate_distance(sLocTime time, uint64_t *val)
* �����������������
* ��ڲ�����
* ���ڲ�������
* ʹ��˵������
********************************************************************************************/
bool caculate_distance(sLocTime time, uint64_t *val)
{
	uint64_t ra, rb, da, db =0;
	uint64_t tmp = 0;
	
	//t1 t4 t5 ʱ����Ч�Լ��
	if(time.t4 < time.t1)
	{
		time.t4 += DW_TIME_OVER;
		time.t5 += DW_TIME_OVER;
	}
	if(time.t5 < time.t4)
	{
		time.t5 += DW_TIME_OVER;
	}
	//t2 t3 t6 ʱ����Ч�Լ��
	
	if(time.t3 < time.t2)
	{
		time.t3 += DW_TIME_OVER;
		time.t6 += DW_TIME_OVER;
	}
	if(time.t6 < time.t3)
	{
		time.t6 += DW_TIME_OVER;
	}

	ra = time.t4 - time.t1;										//Tround1 = T4 - T1  
	rb = time.t6 - time.t3;										//Tround2 = T6 - T3 
	da = time.t5 - time.t4;										//Treply2 = T5 - T4  
	db = time.t3 - time.t2;										//Treply1 = T3 - T2  
	
	if(ra < db)
	{
		goto DError;
	}
//				if(rb < da)
//				{
//					goto DError;
//				}
	if((ra - db)/(rb - da) > 256)
	{
		goto DError;
	}
	
	if(ra * rb <  da * db)
	{
		goto DError;
	}
	else
	{
		tmp = ((ra * rb - da * db) ) / (ra + rb + da + db);		//���㹫ʽ
	}

	*val = tmp * 46903 /100000;
	return true;
DError:
	*val = 0;
	return false;
}

/*******************************************************************************************
* �������ƣ�
* ����������
* ��ڲ�����
* ���ڲ�������
* ʹ��˵������
********************************************************************************************/
void AncWriteSendData(uint8_t *data, uint16_t len)
{
	DwtWriteTxData(data, len);
	gAncDev.txPollNum++;
}

/*******************************************************************************************
* �������ƣ�bool caculate_distance(sLocTime time, uint64_t * val)
* �����������������
* ��ڲ�����
* ���ڲ�������
* ʹ��˵������
********************************************************************************************/
void DwRxDelayErrProcess(void)
{
	gAncDev.devState = UWB_WAIT_STATE;
//	gAncDev.locCount++;
	gAncDev.locErrNum++;
	DWT_Write32BitReg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR_CLE);
//	DWT_RxReset();
}

void DwLocSucProcess(void)
{
	gAncDev.devState = UWB_WAIT_STATE;   //��λ�ɹ�
	gAncDev.locCount++;
}

void WaiteOverErrProcess(void)
{
	gAncDev.waitErr++;
//	DWT_Write32BitReg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR_CLE);
}


/*******************************************************************************************
* �������ƣ�
* ���������������봦����
* ��ڲ�����
* ���ڲ�������
* ʹ��˵������
********************************************************************************************/
void MaxDistProcess(sDistNote *data)
{
	if(gAncDev.maxP != NULL)
	{
		if(data->realDist > gAncDev.maxP->realDist && data->tagId != gAncDev.maxP->tagId)
		{
			gAncDev.maxP = data;
		}
	}
	else
		gAncDev.maxP = data;
}

/*******************************************************************************************
* �������ƣ�
* ����������һ����ǩ���뻺�洦����
* ��ڲ�����
* ���ڲ�������
* ʹ��˵������
********************************************************************************************/
void DistDataStrInit(sDistNote * data, uint16_t tagId)
{
	memset(data, 0x00, sizeof(sDistNote));
	data->tagId = tagId;
}

void WriteDistDataPro(sDistNote *note, uint16_t val)
{
//	if(note->commCnt >= ANC_TOF_REC_MAX) //���� Խ��
//	{
//		DebugErrorProcess();
//	}
//	note->dist[note->commCnt] = val;
//	if(val > note->dist[note->maxPtr])
//		note->maxPtr = note->commCnt;
//	else if(val <  note->dist[note->minPtr])
//		note->minPtr = note->commCnt;
//	note->commCnt++;
//	note->heartCout++;
	
	uint16_t off;
	uint16_t i;
	
	if(note->commCnt > 0)
	{
		off = val > note->realDist ? val - note->realDist : note->realDist - val;
		if(off > 800)
			return;
	}
	
	note->dist[note->wr] = val;
	
	note->commCnt++;
	note->commCnt = note->commCnt < ANC_TOF_REC_MAX ? note->commCnt : ANC_TOF_REC_MAX;
	if(note->wr == note->maxPtr || note->wr == note->minPtr)
	{
		for(i = 0; i < note->commCnt; i++)
		{
			if(note->dist[i] > note->dist[note->maxPtr])
			{
				note->maxPtr = i;
			}
			else if (note->dist[i] < note->dist[note->minPtr])
			{
				note->minPtr = i;
			}
		}
	}
	else
	{
		if(val > note->dist[note->maxPtr])
			note->maxPtr = note->wr;
		else if(val <  note->dist[note->minPtr])
			note->minPtr = note->wr;
	}
	note->wr++;
	note->wr %= ANC_TOF_REC_MAX;
//	note->commCnt++;
//	note->commCnt  = note->commCnt < ANC_TOF_REC_MAX ? note->commCnt : ANC_TOF_REC_MAX;
	note->heartCout++;
	note->right = gRight;  //���Ȩ��
	
//	for(i = 0; i < note->commCnt; i++)
//	{
//		if(note->dist[i] > note->dist[note->maxPtr])
//		{
//			note->maxPtr = i;
//		}
//		else if (note->dist[i] < note->dist[note->minPtr])
//		{
//			note->minPtr = i;
//		}
//	}
		

}

sDistList_N * GetDistDataEqualBuf(uint16_t tagId, uint16_t dist)
{
	sDistList_N * midpoint = NULL;

	for(midpoint = gDistDataBuf.rw_p; midpoint != NULL; midpoint = midpoint->next)
	{
		if(midpoint->p->tagId == tagId)                                //��ǩ��� �ɹ�
			return midpoint;			
	}
	return NULL;
}
	
sDistList_N * GetDistDataBuf(uint16_t tagId, uint16_t dist)
{
	sDistList_N * midpoint;

	for(midpoint = gDistDataBuf.rw_p; midpoint != NULL; midpoint = midpoint->next)
	{
		if(midpoint->p->tagId == tagId)                                //��ǩ��� �ɹ�
			return midpoint;			
	}
	
	if(gDistDataBuf.free_p != NULL)  // û���ҵ�tagid ˵������
	{
		midpoint = gDistDataBuf.free_p;
		gDistDataBuf.free_p = gDistDataBuf.free_p->next;
		midpoint->next = gDistDataBuf.rw_p;
		gDistDataBuf.rw_p = midpoint;
		DistDataStrInit(midpoint->p, tagId);
		return midpoint;
	}
	else  //buf����
	{
		if(gAncDev.maxP != NULL)
		{
			if(dist < gAncDev.maxP->realDist)  //ɾ���滻
			{
				DeleteSecondListData(gAncDev.maxP->tagId);
				DistDataStrInit(gAncDev.maxP, tagId);
				WriteDistDataPro(gAncDev.maxP, dist);
				
				gAncDev.maxP = NULL;
			}
		}
		return NULL;
	}
	
}


void AddDistData(uint16_t tagId, uint16_t val, uint8_t para)
{
	sDistList_N * midpoint;
	
	if(para)
		midpoint = GetDistDataBuf(tagId, val);
	else
		midpoint = GetDistDataEqualBuf(tagId, val);
	if(midpoint != NULL)
	{
		if(para)
		{
			if((midpoint->p->commCnt == 0) && (!midpoint->p->FisrtDistErr) && (val < gUwbRunPara.extent))	//��һ��ͨѶ���ֵ�����ϱ���Χ��ʱ���ݲ��洢
			{
				midpoint->p->FisrtDistErr = 1;
				midpoint->p->ErrDist = val;
			}
			else
			{
				if(midpoint->p->FisrtDistErr)
				{
					midpoint->p->FisrtDistErr = 0;
					WriteDistDataPro(midpoint->p, val);
				}
				WriteDistDataPro(midpoint->p, val);
			}
		}
		else
		{
			if(midpoint->p->commCnt > 0)	
			{
				val = (midpoint->p->wr == 0 ? midpoint->p->dist[ANC_TOF_REC_MAX - 1] : midpoint->p->dist[midpoint->p->wr - 1]);
														//��һ��ͨѶ���ʧ�ܲ����д洢��������������������
				WriteDistDataPro(midpoint->p, val);
			}
		}
	}
}


sDistList_N * GetDistDataPoint(uint16_t tagId)
{
	sDistList_N * midpoint;
	
	for(midpoint = gDistDataBuf.rw_p; midpoint != NULL; midpoint = midpoint->next)
	{
		if(midpoint->p->tagId == tagId)                                //��ǩ��� �ɹ�
			return midpoint;			
	}
	return NULL;	
}

uint16_t GetRelativeRealDist(sDistNote * note)
{
	uint16_t max;
	uint16_t min;
	uint16_t i;
	uint16_t start;
	uint32_t val;

	if(note->commCnt > 0)
	{
		if(note->commCnt < 3)
		{
			return note->dist[note->wr - 1];
		}
		else
		{
			start = (ANC_TOF_REC_MAX + note->wr - 3) % ANC_TOF_REC_MAX;
			min = note->dist[start];
			max = note->dist[start];
			val = min;
			for(i = 1; i < 3; i++)
			{
				if(note->dist[start+i] > max)
					max = note->dist[start+i];
				else if(note->dist[start+i] < min)
					min = note->dist[start+i];
				val += note->dist[start+i];
					
			}
			val = val - min - max;
			return val;
		}
	}
	else if(note->FisrtDistErr == 1)
	{
		return note->ErrDist;
	}
	return 0xffff;
  
}

void SetPollAckData(uint16_t tagId)
{
	sDistList_N * point;
//	uint32_t off;
	uint16_t val;
	
	point = GetDistDataPoint(tagId);
	
	if(point == NULL)
	{
		memset(gTxRespMsg + ANC_DATA_HEAD_SIZE, 0X00, ANC_TX_FRAME_DATA_LEN);
	}
	else
	{
		val = GetRelativeRealDist(point->p);
		if(val == 0xffff)
		{
			memset(gTxRespMsg + ANC_DATA_HEAD_SIZE, 0X00, ANC_TX_FRAME_DATA_LEN);
		}
		else
		{
//		off = point->p->wr > 0 ? point->p->wr -1: ANC_TOF_REC_MAX -1;
			gTxRespMsg[ANC_DATA_HEAD_SIZE] = 1;
			memcpy(gTxRespMsg + ANC_DATA_HEAD_SIZE + 1, &val, 2);		
//		memcpy(gTxRespMsg + ANC_DATA_HEAD_SIZE+1, point->p->dist+ off, 2);	
		}			
	}
//	if(tagId == 388 && *((uint16_t *)(gTxRespMsg + ANC_DATA_HEAD_SIZE+1)) > 100)
//	{
//		gTestErr = 0;
//	}
	
}


sDistList_N * GetDistBufPoint(void)
{
	return gDistDataBuf.rw_p;
}


void DistBufDataCount(sDistNote * data)
{
	int i; 
	uint32_t midVal = 0;
	uint8_t  diff = 0;
	if(data->commCnt < ANC_TOF_MIN_DATA)
	{
//		gAncDev.dataNumErr++;
		for(i = 0; i < data->commCnt; i++)
		{
			midVal += data->dist[i];
		}
		data->realDist = midVal / data->commCnt;
	}
	else
	{
		for(i = 0; i < data->commCnt; i++)
		{
			if(i != data->maxPtr && i != data->minPtr)
				midVal += data->dist[i];
		}
		diff = (data->maxPtr != data->minPtr) ? 2 : 1;
		data->realDist = midVal / (data->commCnt - diff);
	}
//	data->commCnt = 0;
	
	MaxDistProcess(data);
}

void DelDistBufData(sDistList_N * before, sDistList_N * del)
{
	Dist_Delete_List(&gDistDataBuf, before, del);
}



/*******************************************************************************************
* �������ƣ�UWB_Anchor(void *pdata)
* ����������UWB��Ϊ��վʱ��ִ�к���
* ��ڲ�����*pdata
* ���ڲ�������
* ʹ��˵������
********************************************************************************************/
uint32_t GetBeaconAckDelay(void)
{
	uint16_t num = gAncDev.devNum;
	return (num % BEACON_GROUP_NUM) * BEACON_DELAY_SETP + BEACON_DELAY_BASIC;
}

uint64_t SetDelayTxTime(uint64_t start, uint32_t delay)
{
	uint32_t   mid;
	mid = (start + (delay * UUS_TO_DWT_TIME)) >> 8;	
	DWT_SetDelayedTRxTime(mid);																								
	
	return (((uint64_t)(mid & 0xFFFFFFFE)) << 8) + TX_ANT_DLY;
}

void AncIrqDeclareFrameProcess(uint8_t * pdata, uint16_t len)
{
//	INT8U 		err;
//	uint64_t frameRxTime = 0; 
//	uint64_t frameTxTime = 0;
//	uint64_t curTime = 0;
	
	gAncDev.devState = UWB_RX_BEA_STATE;    														//�����豸״̬Ϊ��λ״̬
	gAncDev.beaconNum++;
	gAncDev.currDevId = *((uint16_t *)(pdata + FIRST_ADDR_OFF));          //��ǰ��λ�ı�ǩid��
	
	gTagTime.t2 = GetRxTimeStamp_u64();	
	
	memcpy(gAnc_CommDecResp + FIRST_ADDR_OFF, pdata + FIRST_ADDR_OFF, 2); 	 //����Ӧ���Ŀ�ĵ�ַ
	
	gTagTime.t3 = SetDelayTxTime(gTagTime.t2, GetBeaconAckDelay());
 	
	FinalMsgSetTS(&gAnc_CommDecResp[BEACON_ACK_T2_DIR], gTagTime.t2);					//��T2��T3д�뷢������
	FinalMsgSetTS(&gAnc_CommDecResp[BEACON_ACK_T3_DIR], gTagTime.t3);		

			
	DWT_WriteTxData(sizeof(gAnc_CommDecResp), gAnc_CommDecResp, 0);		//д�뷢������
	DWT_WriteTxfCtrl(sizeof(gAnc_CommDecResp), 0, 1);								  //�趨���ͳ���
	
	gAncDev.txBeaNum++;
//	if(DWT_StartTx(DWT_START_TX_IMMEDIATE) != DWT_SUCCESS)
//	{
//		AncErrorProcess(1);
//	}
	if(DWT_StartTx(DWT_START_TX_DELAYED) != DWT_SUCCESS)    //��ʱ���� ���ʧ��
	{
		DWT_StartTx(DWT_START_TX_IMMEDIATE);			
	}


	gAncDev.devState = UWB_RES_BEA_STATE;  

//	DWT_SetRxTimeOut(0);												//�趨���ճ�ʱʱ�䣬0--û�г�ʱʱ�䣬���޵ȴ�
//	DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//�򿪽���	
	gTxTimeover = 10;
	OSSemPost(gDwSendSem);
}


void RxElseBeaconProcess(uint8_t * pdata, uint16_t len)
{
	DWT_SetRxTimeOut(0);												//�趨���ճ�ʱʱ�䣬0--û�г�ʱʱ�䣬���޵ȴ�
	DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//�򿪽���;
}

void AncRxPollFrameProcess(uint16_t tagId, uint8_t *pdata, uint16_t len)
{
//	uint64_t curTime = 0;
	uint32_t respTxTime;
	
	gAncDev.devState = UWB_RX_POLL_STATE;
	gAncDev.rxPollNum++;

	gAncDev.currDevId = tagId;
	SetFirstAddr(gTxRespMsg, tagId);     //����Resp����֡��ǩ��ַ	

	SetPollAckData(tagId);

	gLocTime.t2 = GetRxTimeStamp_u64();							//���Poll������ʱ��T2
	AncWriteSendData(gTxRespMsg, sizeof(gTxRespMsg));

	DWT_SetRxAfterTxDelay(0);				//���÷�����ɺ��������ӳ�ʱ��
	DWT_SetRxTimeOut(FINAL_RX_TIMEOUT_UUS);				//���ճ�ʱʱ��
//	curTime = GetSysTimeStamp_u64();

	respTxTime = (gLocTime.t2 + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;	  //����Response����ʱ��T3
	DWT_SetDelayedTRxTime(respTxTime);					//����Response����ʱ��T3


//	gAncDev.txPollNum++;
	if(DWT_StartTx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED) != DWT_SUCCESS)    //��ʱ���� ���ʧ��
	{
		DWT_StartTx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
//		AncErrorProcess(5);
		
	}
	gAncDev.devState = UWB_RES_POLL_STATE;
	gTxTimeover = 2;
	OSSemPost(gDwSendSem);
		
	if(len >= 17) //��Э�飬���Ȩ��
	{
		gRight = pdata[14];
	}
	else
		gRight = 3;           //Ĭ��Ȩ��Ϊ3��
}

void AncRxFinalFrameProcess(uint16_t tagId, uint8_t *pdata, uint16_t len)
{
	uint8_t ret = 0;
	uint64_t tof_dtu;
	gAncDev.devState = UWB_RX_FINAL_STATE;
	gAncDev.rxFinalNum++;

	gLocTime.t3 = GetTxTimeStamp_u64();			//���response����ʱ��T3
	gLocTime.t6 = GetRxTimeStamp_u64();			//���final����ʱ��T6

	FinalMsgGetTS_64(&gAncRxBuff[FINAL_MSG_POLL_TX_TS_IDX], &gLocTime.t1);				//�ӽ��������ж�ȡT1��T4��T5
	FinalMsgGetTS_64(&gAncRxBuff[FINAL_MSG_RESP_RX_TS_IDX], &gLocTime.t4);
	FinalMsgGetTS_64(&gAncRxBuff[FINAL_MSG_FINAL_TX_TS_IDX], &gLocTime.t5);
	
	ret = caculate_distance(gLocTime, &tof_dtu);
	if(ret == true)
	{
		gDist = (uint16_t)tof_dtu;
		DwLocSucProcess();
	}
	else
	{
		gAncDev.dataErr++;
	}
	
	AddDistData(tagId, gDist, ret);
	
	gAncDev.devState = UWB_WAIT_STATE;
}


void AncDataFrameProcess(uint8_t *pdata, uint16_t len)
{
	uint16_t tagId;
	
	tagId = *((uint16_t *)(pdata + SECOND_ADDR_OFF));
	
	if(pdata[FRAME_16BIT_ADDR_TYPE_IDX] == FRAME_POLL)								//poll֡
	{	
		AncRxPollFrameProcess(tagId, pdata, len);
	}
	else if (pdata[FRAME_16BIT_ADDR_TYPE_IDX] == FRAME_FINAL)
	{
		if(tagId == gAncDev.currDevId)
		{
			AncRxFinalFrameProcess(tagId, pdata, len);
		}
		else
		{
			gAncDev.idErr++;
		}
		DWT_SetRxTimeOut(0);								//�趨���ճ�ʱʱ�䣬0--û�г�ʱʱ�䣬���޵ȴ�
		DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//�򿪽���
	}
	else
	{
		RestartRxProcess();   //����֡�Ĵ�����
	}	
}

static void tx_conf_cb(const dwt_cb_data_s *cb_data)
{
	if(gAncDev.devState == UWB_RES_BEA_STATE)
	{
		OSSemPost(gDwSendOkSem);
		DWT_SetRxTimeOut(0);								//�趨���ճ�ʱʱ�䣬0--û�г�ʱʱ�䣬���޵ȴ�
		DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//�򿪽���;
	}
	else
	{
		OSSemPost(gDwSendOkSem);
	}
}


static void rx_to_cb(const dwt_cb_data_s *cb_data)
{
	if(gAncDev.devState == UWB_RES_POLL_STATE)
	{
		gAncDev.waitErr++;
		gAncDev.devState = UWB_WAIT_STATE;
	}
	DWT_SetRxTimeOut(0);								//�趨���ճ�ʱʱ�䣬0--û�г�ʱʱ�䣬���޵ȴ�
	DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//�򿪽���
}

static void rx_err_cb(const dwt_cb_data_s *cb_data)
{
	DWT_SetRxTimeOut(0);								//�趨���ճ�ʱʱ�䣬0--û�г�ʱʱ�䣬���޵ȴ�
	DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//�򿪽���
}

static void else_err_cb(const dwt_cb_data_s *cb_data)
{
	gAncDev.elseErr++;
	DWT_ForceTRxOff(); // Turn the RX off
	DWT_RxReset(); // Reset in case we were late and a frame was already being received
	DWT_SetRxTimeOut(0);								//�趨���ճ�ʱʱ�䣬0--û�г�ʱʱ�䣬���޵ȴ�
	DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//�򿪽���
}


static void UwbRxOkCb(const dwt_cb_data_s *cb_data)
{
	FrameConField frameCon;
#if UWB_DEBUG_ENABLE == 1
	SetDebugCount();
#endif /*UWB_DEBUG_ENABLE == 1*/
	
	if (cb_data->datalength <= ANC_RX_LEN_MAX)
	{
		DWT_ReadRxData(gAncRxBuff, cb_data->datalength, 0);		//��ȡ��������
		frameCon.frameControl = *((uint16_t *)gAncRxBuff);
			
		if(frameCon.FrameConBit.sorcAddrMode == ID_LENGTH_16)    //16λ��ַ
		{
			switch (frameCon.FrameConBit.frameType)
			{
				case BEACON_FRAME:		//�ű�֡
				{
					if(gAncRxBuff[BEACON_FRAME_TYPE_IDX] == FRAME_COMM_DEC) //��λ�ű�֡����
					{
						AncIrqDeclareFrameProcess(gAncRxBuff, cb_data->datalength);
					}
					else
					{
						RxElseBeaconProcess(gAncRxBuff, cb_data->datalength);//�����ű�֡����
					}
				}
				break;
					
				case DATA_FRAME:		//����֡����
				{
//					if(gAncDev.devState == UWB_LOCATION_STATE)     //���ڽ��б�ǩ��λ����
					{
						if(*((uint16_t *)(gAncRxBuff + FIRST_ADDR_OFF)) == gAncDev.devNum)  //����֡�Ƿ����Լ���
						{
//							gAncDev.rxDataNum++;
							AncDataFrameProcess(gAncRxBuff, cb_data->datalength);
						}
						else //���󣬲�Ӧ�õ������� ��������֡
						{
							AncErrorProcess(7);
						}
					}
				}
				break;

				default:
					AncErrorProcess(10);//
				break;
				}
		}
		else
		{
			AncErrorProcess(8);//����ĵ�ַ ��ʽ
		}
	}
	 else
	 {
		 AncErrorProcess(9);//
	 }
}


void AnchorIdInit(void)
{
	gAncDev.devNum = GetUwbId();
	DWT_SetAddress16(gAncDev.devNum);					//����֡���˵�ַ

	SetAnchorAddr(gAnc_CommDecResp, gAncDev.devNum);	//���ö�λ����֡��Ӧ��֡��վ��ַ
	SetAnchorAddr(gTxRespMsg, gAncDev.devNum);			//����Resp����֡��վ��ַ
}
/*******************************************************************************************
* �������ƣ�void CanRxParaProcess(void)
* ����������UWB�����յ���������֡������UWBģ����Ϣ
* ��ڲ�����none
* ���ڲ�����none
* ʹ��˵������
********************************************************************************************/
void CanRxParaProcess(void)
{
	AnchorIdInit();
}

void UwbAnchorIrqTask(void *pdata)
{
	INT8U err;
//	uint16_t frameLen = 0;
//	FrameConField frameCon;
	
	gDwSendSem = OSSemCreate(0);
	gDwSendOkSem = OSSemCreate(0);
	
	Dist_ListDataInit(&gDistDataBuf);
#if UWB_DEBUG_ENABLE == 1
	CreatUwbDebugTask();
#endif /*UWB_DEBUG_ENABLE == 1*/
#if CAN_TASK_MODE == CAN_POSITION_MODE	
	WaitUwbStartSem();   //�ȴ�������Ϣ
#endif 	
	gDwAnchorConfig.eui = DWT_GetPartID();
	Dw1000InitConfig(&gDwAnchorConfig);	
//	
	gAncDev.devNum = GetUwbId();		//֧�ܿ��������
	
//	gAncDev.devNum = 239;
//	gUwbRunPara.extent = 500;
//	gUwbRunPara.interval = 20;
//	gUwbState = UWB_NORMAL;
	
	DWT_SetAddress16(gAncDev.devNum);		//����֡���˵�ַ

	SetAnchorAddr(gAnc_CommDecResp, gAncDev.devNum);	//���ö�λ����֡��Ӧ��֡��վ��ַ
	SetAnchorAddr(gTxRespMsg, gAncDev.devNum);			//����Resp����֡��վ��ַ
	
	LocationDataAppInit();
	
	dwt_setcallbacks(&tx_conf_cb, &UwbRxOkCb, &rx_to_cb, &rx_err_cb, &else_err_cb);
	DWT_SetAutoRxReEnable(ENABLE);	
	DWT_EnableFrameFilter(SYS_CFG_FF_ALL_EN);						//����֡����
	
	DWT_SetRxTimeOut(0);											//�趨���ճ�ʱʱ�䣬0--û�г�ʱʱ�䣬���޵ȴ�
	DWT_SetInterrupt(DWT_INT_ALL | DWT_INT_TFRS, ENABLE);			//�����ж�
//	DWT_SetInterrupt(DWT_INT_ALL_CLE|DWT_INT_TFRS, ENABLE);
	DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//�򿪽���
	
	for (;;)
	{
		OSSemPend(gDwSendSem, 0, &err);
		OSSemPend(gDwSendOkSem, gTxTimeover, &err);
		if (err == OS_ERR_TIMEOUT)				//����ʧ��
		{
			port_DisableEXT_IRQ();								//�ر��ж�
			DWT_ForceTRxOff();
				
			DWT_RxReset();
			DWT_SetRxTimeOut(0);								//�趨���ճ�ʱʱ�䣬0--û�г�ʱʱ�䣬���޵ȴ�
			DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//�򿪽���
			port_EnableEXT_IRQ();								//ʹ���ж�
		}
	}
}




