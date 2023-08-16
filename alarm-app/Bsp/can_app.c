/*
*********************************************************************************************************
*	                                            UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : can_app.c
*    Module  : can driver
*    Version : V1.0
*    History :
*   -----------------
*              can app 
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/

#include "can_app.h"
#include "CanIap.h"
#include "app_cfg.h"

#include "logic.h"

#include "angle_sensor.h"

#include "main.h"

/********************************************************************************
*��������
*********************************************************************************/

OS_STK CanRx_stk[CANRX_TASK_SIZE];
OS_STK CanTx_stk[CANTX_TASK_SIZE];

uint32_t gUwbState = 0;				//UWB��Ϣ��ʼ����״̬,=0��ʾδ�������ã�=1��ʾ�������

sCAN_BUFFER gUwbCanTxBuf;
sCAN_FRAME gCanRxcData;

#if CAN_TASK_MODE == CAN_MONITOR_MODE
sUWB_RUN_INFO 	gUwbRunPara= {5,0,0,100, 0};
#else
sUWB_RUN_INFO 	gUwbRunPara;
#endif /* CAN_TASK_MODE == CAN_MONITOR_MODE*/

OS_EVENT *gUwbStart;	

uint32_t gACkframe;
uint8_t gTtl;
static u8 s_u8CRC_TABLE[] = {0x00,0x31,0x62,0x53,0xC4,0xF5,0xA6,0x97,0xB9,0x88,0xDB,0xEA,0x7D,0x4C,0x1F,0x2E};	 //CRC_TABLE

uint16_t gWlAddr = 0;				//WLM���õ�ַ

/*******************************************************************************************
**�������ƣ�CRC_8(u8 *PData, u8 Len)
**�䡡�룺None
**�䡡����None
**����������Can��������
*******************************************************************************************/
u8 CRC_8(u8 *PData, u8 Len)
{
	u8	CRC_Temp = 0;
	u8	Temp, i;
	u8	PData_H = 0;
	u8	PData_L = 0;

	for (i = 0; i < Len; i++)
	{
		PData_L = PData[i];
		if (i < (Len - 1))
		{
			PData_H = PData[i + 1];

			Temp = CRC_Temp >> 4;
			CRC_Temp <<= 4;
			CRC_Temp ^= s_u8CRC_TABLE[Temp ^ (PData_H >> 4)];
			Temp = CRC_Temp >> 4;
			CRC_Temp <<= 4;
			CRC_Temp ^= s_u8CRC_TABLE[Temp ^ (PData_H & 0x0F)];
			i ++;
		}

		Temp = CRC_Temp >> 4;
		CRC_Temp <<= 4;
		CRC_Temp ^= s_u8CRC_TABLE[Temp ^ (PData_L >> 4)];
		Temp = CRC_Temp >> 4;
		CRC_Temp <<= 4;
		CRC_Temp ^= s_u8CRC_TABLE[Temp ^ (PData_L & 0x0F)];
	}

	return (CRC_Temp);
}

/**********************************************************************************************************
* �������������ͽǶ�����֡
 * �����������
 * ����ʱ�䣺
 * �������ߣ�
**********************************************************************************************************/
sCAN_FRAME SendAngleData(uint32_t func, uint8_t dest, uint8_t *data, uint8_t size)
{
	u16 u16VauleTemp;
	sCAN_FRAME ret;
	CanHeadID  SendFrame;

	memset(&SendFrame, 0x00, sizeof(SendFrame));
	memset(&ret, 0x00, sizeof(ret));
	switch (func)
	{
		case ANGLE_REPORT_VALUE:			//�Ƕ��ϱ�֡
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_REPORT_VALUE;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			SendFrame.ID.SN = u16CanLeftLiuShuiNumb;
			u16CanLeftLiuShuiNumb ++;
			u16CanLeftLiuShuiNumb %= 16; 
			ret.Stdid = SendFrame.u32Id;
		
			ret.DLC = CAN_LENGTH_6;
			ret.Data[0x00] = 0x00;
			ret.Data[0x01] = SUB_DEVICE_NUM;
			LogicRunInfApi(LOGIC_GET_ANGLEVALUE_X, &u16VauleTemp);
			ret.Data[0x02] = (u8)(u16VauleTemp & 0xFF);
			ret.Data[0x03] = (u8)((u16VauleTemp & 0xFF00) >> 8);
			LogicRunInfApi(LOGIC_GET_ANGLEVALUE_Y, &u16VauleTemp);
			ret.Data[0x04] = (u8)(u16VauleTemp & 0xFF);
			ret.Data[0x05] = (u8)((u16VauleTemp & 0xFF00) >> 8);
			break;
			
		case ANGLE_SET_DEV_TYPE:			//�����豸����/Ӧ��
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_SET_DEV_TYPE;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			ret.Stdid = SendFrame.u32Id;
			ret.DLC = CAN_LENGTH_8;
		
			ret.Data[0] = data[0];
			ret.Data[1] = data[1];
			ret.Data[2] = data[2];
			ret.Data[3] = data[3];
			ret.Data[4] = data[4];
			ret.Data[5] = data[5];
			ret.Data[6] = data[6];
			ret.Data[7] = data[7];
			break;
			
		case ANGLE_CHECK_VALUE:					//�ǶȲ�ѯ֡
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_REPORT_VALUE;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			ret.Stdid = SendFrame.u32Id;
		
			ret.DLC = CAN_LENGTH_6;
			ret.Data[0x00] = 0x00;
			ret.Data[0x01] = SUB_DEVICE_NUM;
			LogicRunInfApi(LOGIC_GET_ANGLEVALUE_X, &u16VauleTemp);
			ret.Data[0x02] = (u8)(u16VauleTemp & 0xFF);
			ret.Data[0x03] = (u8)((u16VauleTemp & 0xFF00) >> 8);
			LogicRunInfApi(LOGIC_GET_ANGLEVALUE_Y, &u16VauleTemp);
			ret.Data[0x04] = (u8)(u16VauleTemp & 0xFF);
			ret.Data[0x05] = (u8)((u16VauleTemp & 0xFF00) >> 8);
			break;
		
		case ANGLE_SET_MODIFY_PARAM_NUMB:		//���ó�������ֵ����
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_SET_MODIFY_PARAM_NUMB;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			ret.Stdid = SendFrame.u32Id;
			ret.DLC = CAN_LENGTH_6;
		
			ret.Data[0] = data[0];
			ret.Data[1] = data[1];
			ret.Data[2] = data[2];
			ret.Data[3] = data[3];
			ret.Data[4] = data[4];
			ret.Data[5] = data[5];
			break;
			
		case ANGLE_GET_MODIFY_PARAM_NUMB_ACK:	
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_GET_MODIFY_PARAM_NUMB_ACK;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			ret.Stdid = SendFrame.u32Id;
		
			ret.DLC = CAN_LENGTH_6;
		
			ret.Data[0x00] = 0x00;
			ret.Data[0x01] = 0x00;
			ret.Data[2] = data[2];
			ret.Data[3] = data[3];
			ret.Data[4] = data[4];
			ret.Data[5] = data[5];
			break;
			
		case ANGLE_GET_DEV_TYPE_ACK:		//��ȡ�豸����Ӧ��
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_GET_DEV_TYPE_ACK;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			ret.Stdid = SendFrame.u32Id;
			ret.DLC = CAN_LENGTH_8;
		
			ret.Data[0] = data[0];
			ret.Data[1] = data[1];
			ret.Data[2] = data[2];
			ret.Data[3] = data[3];
			ret.Data[4] = data[4];
			ret.Data[5] = data[5];
			ret.Data[6] = data[6];
			ret.Data[7] = data[7];
		break;
		
		case ANGLE_SET_MODIFY_VALUE:			//���ó�������ֵӦ��
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_SET_MODIFY_VALUE;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			ret.Stdid = SendFrame.u32Id;
			ret.DLC = CAN_LENGTH_8;
		
			ret.Data[0] = data[0];
			ret.Data[1] = data[1];
			ret.Data[2] = data[2];
			ret.Data[3] = data[3];
			ret.Data[4] = data[4];
			ret.Data[5] = data[5];
			ret.Data[6] = data[6];
			ret.Data[7] = data[7];
			break;
			
		case ANGLE_GET_MODIFY_VALUE_ACK:			//�ض���������ֵӦ��
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_GET_MODIFY_VALUE_ACK;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			ret.Stdid = SendFrame.u32Id;
			ret.DLC = CAN_LENGTH_8;
		
			ret.Data[0] = data[0];
			ret.Data[1] = data[1];
			ret.Data[2] = data[2];
			ret.Data[3] = data[3];
			ret.Data[4] = data[4];
			ret.Data[5] = data[5];
			ret.Data[6] = data[6];
			ret.Data[7] = data[7];
		break;

		case ANGLE_MODIFY_PARAM_SAVE:			//��������ֵ��������
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_MODIFY_PARAM_SAVE;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			ret.Stdid = SendFrame.u32Id;
			ret.DLC = CAN_LENGTH_2;
		
			ret.Data[0] = data[0];
			ret.Data[1] = data[1];
			break;

		case ANGLE_GET_WORK_PARAM_ACK:
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_GET_WORK_PARAM_ACK;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			ret.Stdid = SendFrame.u32Id;
			ret.DLC = CAN_LENGTH_8;
		
			ret.Data[0] = 0x00;
			LogicRunInfApi(LOGIC_GET_WORK_MODE, &u16VauleTemp);			//����ģʽ
			ret.Data[1] = (u8)(u16VauleTemp & 0xFF);
			LogicRunInfApi(LOGIC_GET_REPORT_INTERVAL, &u16VauleTemp);		//�Ƕ��ϱ����
			ret.Data[2] = (u8)(u16VauleTemp & 0xFF);
			ret.Data[3] = (u8)((u16VauleTemp & 0xFF00) >> 8);
			ret.Data[4] = 0x00;
			ret.Data[5] = 0x00;
			ret.Data[6] = 0x00;
			ret.Data[7] = 0x00;
		break;
		
		case ANGLE_READ_VERSION_ACK:		//�ض��汾��Ӧ��
			SendFrame.ID.RID = dest;
			SendFrame.ID.TID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame.ID.FT = ANGLE_READ_VERSION_ACK;
			SendFrame.ID.SUM = 0;
			SendFrame.ID.SUB = 0;
			SendFrame.ID.ACK = NO_ACK;
			ret.Stdid = SendFrame.u32Id;
			ret.DLC = CAN_LENGTH_8;
		
			ret.Data[0] = 0x00;
			ret.Data[1] = (u8)(ANGLE_DEV_TYPE & 0xFF);
			ret.Data[2] = 0x00;
			ret.Data[3] = 0x00;
			ret.Data[4] = 0x00;
			ret.Data[5] = VERSION_1;   //�汾��
			ret.Data[6] = VERSION_2;   //�汾��
			ret.Data[7] = VERSION_3;   //�汾��
		break;
		
		default:
			break;
	}
	return ret;
		
}
/************************************************************************************************
* ��������������UWB����֡
 * �����������
 * ����ʱ�䣺
 * �������ߣ�
*************************************************************************************************/
sCAN_FRAME SendUwbData(uint32_t func, uint8_t dest, uint8_t *data, uint8_t size)
{
	CanHeadID  tmp;
	sCAN_FRAME ret;
	uint8_t    buf[12];

	memset(&tmp, 0x00, sizeof(tmp));
	memset(&ret, 0x00, sizeof(ret));
	switch (func)
	{
		case UWB_CONFIG_REQ:			//��������
		case UWB_HEART:					//��Ա��λģ������
			tmp.ID.TID = UWB_ID;
			memcpy(ret.Data, "ALM", 3);
			ret.Data[3] = UWB_MODEL;          //ָ��Ϊ��Ա��λģ��
			ret.Data[4] = (func == UWB_CONFIG_REQ) ? 1 : 0;
			ret.Data[5] = VERSION_1;   //�汾��
			ret.Data[6] = VERSION_2;   //�汾��
			ret.Data[7] = VERSION_3;   //�汾��
			ret.DLC = 8;
			ret.Stdid = tmp.u32Id;
			break;

		case UWB_INFO_REPORT:			//1����Ա��λ��Ϣ�ϱ�
			if(gTtl >= 0x10)
				gTtl = 0;
			tmp.ID.TID = UWB_ID;
			tmp.ID.FT = 1;
			tmp.ID.SN = gTtl;
			gTtl++;
			memcpy(ret.Data, data, size);
//				ret.Data[7] = CRC_8(ret.Data,  7);
			ret.DLC = 8;
			ret.Stdid = tmp.u32Id;
		
			memcpy(buf, &ret.Stdid, 4);
			buf[4] = 8;
			memcpy(buf + 5, ret.Data, 7);
			ret.Data[7] = CRC_8(buf, 12);
			break;

		default:
			break;
	}
	return ret;
		
}

/************************************************************************************************
* �������������ͱ�ǩ��Ϣ
 * �����������
 * ����ʱ�䣺
 * �������ߣ�
*************************************************************************************************/
int32_t WriteReportInfo(uint8_t tagNum, uint16_t tagId, uint16_t dist, uint16_t state, uint8_t right)
{
	sTag_Info 	tmp;
	sCAN_FRAME  cantmp;
	
	memset(&tmp, 0x00, sizeof(tmp));
	tmp.peoplenum = tagNum;
	tmp.tagInfo.sTag.tagState = state;
	tmp.tagInfo.sTag.author = right;    //���Ȩ��
	tmp.tagId = tagId;
	tmp.tagDist = dist;
	
	cantmp = SendUwbData(UWB_INFO_REPORT, 0, (uint8_t *)&tmp, 6);
	return CanBufferWrite(&gUwbCanTxBuf, cantmp);

}

/*******************************************************************************************
**�������ƣ�void CanTx_Task(void *p_arg)
**�䡡�룺None
**�䡡����None
**����������Can��������
*******************************************************************************************/
#if CAN_TASK_MODE == CAN_POSITION_MODE
void CanTx_Task(void *p_arg)
{
    INT8U err;
    sCAN_FRAME sendData;
    CanBufferDataInit(&gUwbCanTxBuf);
    
    while (1)
    {
        if (gUwbState == UWB_INIT)			//��Ա��λģ���ʼδ����״̬
        {
            STM32_CAN_Write(0, SendUwbData(UWB_CONFIG_REQ, 0, NULL, 0), CAN_FF_EXTENDED);
            OSTimeDly(REQ_CONFIG_DELAY);			//���ò��ɹ���ÿ���500ms����һ�β���
        }
        else								//��Ա��λģ���ʼ�������
        {
            OSSemPend(gUwbCanTxBuf.dataSem, UWB_HEART_DELAY, &err);
            if (err == OS_ERR_NONE)
            {
                if (CanBufferRead(&gUwbCanTxBuf, &sendData) == 0)
                {
                    while (STM32_CAN_Write(0, sendData, CAN_FF_EXTENDED) == 1)
                    {
                        OSTimeDly(5); // д��������
                    }
                }
                else//can����buf��
                {
                    gUwbState= UWB_INIT;
                }
            }
            else if (err == OS_ERR_TIMEOUT)
            {
                STM32_CAN_Write(0, SendUwbData(UWB_HEART, 0, NULL, 0), CAN_FF_EXTENDED); // д��Ա��λ����֡
            }
            else
            {
                ;//
            }
        }
    }
}

#else

//	void CanTx_Task(void *p_arg)
//	{
//		INT8U  	        	err;

//		sCAN_FRAME        sendData;
//	//  OS_CPU_SR       cpu_sr = 0;
//		
//		CanBufferDataInit(&gUwbCanTxBuf);

//		while(1)
//		{
//			OSSemPend(gUwbCanTxBuf.dataSem, UWB_HEART_DELAY, &err);
//			if( err == OS_ERR_NONE)
//			{
//				if(CanBufferRead(&gUwbCanTxBuf, &sendData) == 0)
//				{
//					while(STM32_CAN_Write(0, sendData, CAN_FF_EXTENDED)) // д��������
//					{
//						OSTimeDly(1);
//					}
//				}
//				else{//can����buf��
//					gUwbState= UWB_INIT;
//					
//				}
//			}
//		}
//	}
#endif /*CAN_TASK_MODE == CAN_POSITION_MODE*/

void WaitUwbStartSem(void)
{
	INT8U err;
	OSSemPend(gUwbStart, 0, &err);
}


uint16_t GetUwbId(void)
{
	return gUwbRunPara.standNum;			//֧�ܿ��������
}

__inline uint16_t GetUwbReportTime(void)
{
	uint16_t tmp;
	tmp = gUwbRunPara.interval * 100;
	return tmp;
}
/*******************************************************************************************
**�������ƣ�int32_t CheckTagState(uint16_t dist)
**�䡡�룺��ǩ����
**�䡡����1���뿪��ⷶΧ 0���ڼ�ⷶΧ��
**����������
*******************************************************************************************/
int32_t CheckTagState(uint16_t dist)
{
	return (dist > gUwbRunPara.extent) ? 1 : 0;
}


uint16_t GetWLMId(void)
{
	return gWlAddr;			//֧�ܿ��������
}
can_trasnmit_message_struct WLTxCan;
/***********************************************************************************************
** �� �� ����	CanRcvIrMatchProc
** �䡡  �룺	
** �䡡  ����	
** ����������	����֧�ܿ������ĺ�����������ݣ�ת��ң������WL�ĺ���ͨѶЭ��֡
** ע�����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
static u32 CanRcvIrMatchProc(sCanFrame *sRxCan)
{
	u32 u32RetVal = SUCCESS,i;
	static u16 LastWlAddr = 0, WlAddr;
	IR_INFO_u	data;
	sCanFrame sTxCan;
	
	//�����ҪӦ����Ӧ��֡
	if(sRxCan->u32ID.ID.ACK == eACK)
	{
		sTxCan.u32ID.ID.ACK = eNOACK;
		sTxCan.u32ID.ID.FT = sRxCan->u32ID.ID.FT;
		sTxCan.u32ID.ID.RD = sRxCan->u32ID.ID.RD;
		sTxCan.u32ID.ID.RID = sRxCan->u32ID.ID.TID;
		sTxCan.u32ID.ID.SN =  sRxCan->u32ID.ID.SN;
		sTxCan.u32ID.ID.SUB = sRxCan->u32ID.ID.SUB;
		sTxCan.u32ID.ID.SUM = sRxCan->u32ID.ID.SUM;
		sTxCan.u32ID.ID.TID = sRxCan->u32ID.ID.RID;
		
		sTxCan.u16DLC = sRxCan->u16DLC;
		
		for(i=0;i<sTxCan.u16DLC;i++)
		{
			sTxCan.u8Data[i] = sRxCan->u8Data[i];
		}
		
		WLTxCan.tx_efid = sTxCan.u32ID.u32Id;
		WLTxCan.tx_ff = CAN_FF_EXTENDED;
		WLTxCan.tx_dlen = sTxCan.u16DLC;
		for(i = 0; i < sTxCan.u16DLC; i++)
		{
			WLTxCan.tx_data[i] = sRxCan->u8Data[i];
		}
		can_message_transmit(CAN0, &WLTxCan);
		//InsCanTrsQueue(&sTxCan, TRUE);
	}
	
	WlAddr = ((u16)sRxCan->u8Data[4] << 8) + sRxCan->u8Data[3];//����ܺ�
	data.sIrInfo.Type = 2;									//��������
	data.sIrInfo.ScNoLSB3 = (((u32)sRxCan->u8Data[4] << 8) + sRxCan->u8Data[3]) & 0x7;//����ܺŵ�3λ
	data.sIrInfo.Sign1 = 1;									//��ʼ��ʶ
	data.sIrInfo.ScNoMSB6 = ((((u32)sRxCan->u8Data[4] << 8) + sRxCan->u8Data[3]) >> 3) & 0x3f;//����ܺŵ�6λ
	data.sIrInfo.Dir = eDirectHS;							//���ͷ���
	data.sIrInfo.Sign2 = 0;									//�м��ֽڱ�ʶ
	data.sIrInfo.Result = sRxCan->u8Data[2] & 0x01;			//������
	data.sIrInfo.ACK = eACK;								//Ӧ���ʶ
	data.sIrInfo.RemoteID = sRxCan->u8Data[5] & 0xf;		//ң����ID
	data.sIrInfo.ScNoIncDir = sRxCan->u8Data[1] & 0x01;		//֧�ܿ������ܺ�����
	data.sIrInfo.Sign3 = 0;									//�м��ֽڱ�ʶ

	// �������������ɹ���������ɫָʾ�ƣ�
	if(data.sIrInfo.Result == 0x01)
	{
		SaveSRemoteCtrlInfo((u8)data.sIrInfo.RemoteID);		//����ң����ID
//		SetLedState(LED_GREEN,TRUE,0xff);
	}
	
	// �յ���֧����Ϊ���ߺ����շ�ģ������ߵ�ַ
	//LastWlAddr = u16GetPTU16(PT_SER_SC_NO);
	if(LastWlAddr != WlAddr)
	{
//		SetWlAddr((WlAddr == 510) ? 0xff : (u8)(WlAddr));
        SetWlAddr((u8)(WlAddr));
		LastWlAddr = WlAddr;
		
//		gWlAddr = WlAddr;
	}
	
	//Send_InfraredData((u16)((data.u8IrInfo[1] << 8) | data.u8IrInfo[0]),data.u8IrInfo[2]);
		
	return u32RetVal;
}
/***********************************************************************************************
** �� �� ����	CanRcvWlSendProc
** �䡡  �룺	
** �䡡  ����	
** ����������	����֧�ܿ����������߶������ݣ�ת��ң������ wl ������ͨѶЭ��֡
** ע�����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 CanRcvWlSendProc(sCanFrame *sRxCan, u8 To)
{
	u32 u32RetVal = SUCCESS;
	u8 i;
	st_WL_TRANS_DATA st_WlTrs;	
	sCanFrame sTxCan;
	
	memset(&st_WlTrs,0,sizeof(st_WlTrs));
	memset(&sTxCan,0,sizeof(sTxCan));
	//�����ҪӦ����Ӧ��֡
	if(sRxCan->u32ID.ID.ACK == eACK)
	{
		sTxCan.u32ID.ID.ACK = eNOACK;
		sTxCan.u32ID.ID.FT =  sRxCan->u32ID.ID.FT;
		sTxCan.u32ID.ID.RD =  sRxCan->u32ID.ID.RD;
		sTxCan.u32ID.ID.RID = sRxCan->u32ID.ID.TID;
		sTxCan.u32ID.ID.SN =  sRxCan->u32ID.ID.SN;
		sTxCan.u32ID.ID.SUB = sRxCan->u32ID.ID.SUB;
		sTxCan.u32ID.ID.SUM = sRxCan->u32ID.ID.SUM;
		sTxCan.u32ID.ID.TID = sRxCan->u32ID.ID.RID;
		
		sTxCan.u16DLC = sRxCan->u16DLC;
		
		for(i = 0; i < sTxCan.u16DLC; i++)
			sTxCan.u8Data[i] = sRxCan->u8Data[i];
		
		WLTxCan.tx_efid = sTxCan.u32ID.u32Id;
		WLTxCan.tx_ff = CAN_FF_EXTENDED;
		WLTxCan.tx_dlen = sTxCan.u16DLC;
		for(i = 0; i < sTxCan.u16DLC; i++)
		{
			WLTxCan.tx_data[i] = sRxCan->u8Data[i];
		}
		can_message_transmit(CAN0, &WLTxCan);
		//InsCanTrsQueue(&sTxCan, TRUE);
	}
	//���͸�ң����
// 	st_WlTrs.u8Addr = GetRemoteIdByWl();		// ��ȡң����ID
	st_WlTrs.u8Addr = 255 - To;					// ��ȡң�������յ�ַ
	st_WlTrs.u8Ack = sRxCan->u32ID.ID.ACK;
	st_WlTrs.u8FT = sRxCan->u32ID.ID.FT;
	st_WlTrs.u32Len = sRxCan->u16DLC + 1 + 4;	// 1��ʾCAN����֡�ĳ���ռ��һ���ֽڣ�4��ʾCAN IDռ���ĸ��ֽ�
	
	for (i = 0; i < 4 ; i++)
	{
		st_WlTrs.u8WlData[i] = ((sRxCan->u32ID.u32Id >> (i << 3)) & 0xff);
	}
	
	st_WlTrs.u8WlData[i] = sRxCan->u16DLC;
	
	for (i = 5; i < st_WlTrs.u32Len; i++)
	{
		st_WlTrs.u8WlData[i] = sRxCan->u8Data[i-5];
	}
	
	//InsWlTxBuf(&st_WlTrs, TRUE);
	WL_SendData(st_WlTrs.u8Addr, st_WlTrs.u8WlData, st_WlTrs.u32Len);
//	SetLedState(LED_CAN_TRANS,TRUE,3);
	
	// ������߶������ɹ���������ɫָʾ�ƣ�
	//if(sRxCan->u8Data[2] == 0x01 && (sRxCan->u32ID.ID.FT == FT_SC_TO_WL_RF_MATCH_RST))
	if((sRxCan->u8Data[2] & WL_MATCH_RESULT_MASK) == WL_MATCH_OK && (sRxCan->u32ID.ID.FT == FT_SC_TO_WL_RF_MATCH_RST))
	{
		SaveSRemoteCtrlInfo((u8)To);			//����ң����ID
//		SetLedState(LED_GREEN,FALSE,0x00);
//		SetLedState(LED_RED,TRUE,0xff);
	}
	
	// ������֧�ܿ������������Ķ���������֡��Ϩ��ָʾ��
	if(sRxCan->u32ID.ID.FT == FT_SC_TO_WL_DISCONNECT)
	{
//		SetLedState(LED_GREEN,FALSE,0x00);
//		SetLedState(LED_RED,FALSE,0x00);
	}
	return u32RetVal;
}
/***********************************************************************************************
** �� �� ����	CanRcvSetPar
** �䡡  �룺	
** �䡡  ����	
** ����������	����֧�ܿ��������ò���֡��������Ӧ����
** ע�����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
static u32 CanRcvSetPar(sCanFrame *sRxCan)
{
//	u32 u32RetVal = TRUE;
//	u16 u16Offset;
//	u32 i,k;
	uint8_t SC_RFID_SYNC0 = 0;
	uint8_t SC_RFID_SYNC1 = 0;
	u32 u32SN = 0xffffffff;			//�ϵ��ʼ�����к�
	
	if(u32SN == (u32)sRxCan->u32ID.ID.SN)
		return FALSE;
	else
		u32SN = sRxCan->u32ID.ID.SN;
    

	/*
	 * ���üܺ�
	 */
    SetWlAddr((u8)sRxCan->u8Data[0]);
	RfidWriteReg(CC1101_ADDR, gWlAddr);
	/*
	 * ��������ͬ����
	 */
	SC_RFID_SYNC0 = (u8)sRxCan->u8Data[1];
	RfidWriteReg(CC1101_SYNC0, SC_RFID_SYNC0); // SYNC0
	SC_RFID_SYNC1 = (u8)sRxCan->u8Data[2];
	RfidWriteReg(CC1101_SYNC1, SC_RFID_SYNC1); // SYNC
    
    
	
//	u16Offset = (u16)sRxCan->u8Data[0] | (u16)(sRxCan->u8Data[1] << 8);
	
	/*
	 * ��������ͬ����
	 */
//	if(u16Offset == WL_SYNC)
//	{
//		SetSync((u32)sRxCan->u8Data[2]|(u32)(sRxCan->u8Data[4]<<8));
//		return TRUE;
//	}

//	k = (sRxCan->u16DLC - 2) >> 1;			// ʵ�ʲ�������

//	for(i = 0;i < k;i++)
//	{
//		FlushPTPar(u16Offset + i,(u16)sRxCan->u8Data[2 + (i<<1)], (u16)sRxCan->u8Data[3 + (i<<1)]);
//	}
	
//	gWlAddr = (u32)sRxCan->u8Data[2] | (u32)(sRxCan->u8Data[4] << 8);
	/*
	 * ���üܺţ�����Ƿ�����ָ��Ϊ0xFF
	 */
//	if(u16Offset == PT_SER_SC_NO)//u16GetPTU16(PT_SER_SC_NO)
//	{
//		//SetWlAddr((u16GetPTU16(PT_SER_SC_NO) == 510)?0xFF:(u8)u16GetPTU16(PT_SER_SC_NO));
//		SetWlAddr((gWlAddr == 510) ? 0xFF : (u8)gWlAddr);
//		return TRUE;
//	}
	return TRUE;
}
/***********************************************************************************************
** �� �� ����	CanRcvBusProc
** �䡡  �룺	
** �䡡  ����	
** ����������	����֧�ܿ�����������CAN��������
** ע�����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
static st_WL_TRANS_DATA st_WlTrsTmp;						//������д�����������
static u32 u32WlBusDataLen = 0;							    //��¼����������������ݸ���
static u32 u32CanToWlFrmNum = 0;							//Ҫת��Ϊ�������ݵ�CAN����֡����
static u32 u32CurFrm = 0;									//��¼��ǰ֡������һ֡�Ƚ�
static u32 CanRcvBusProc(sCanFrame *sRxCan)
{
//	u32 u32RetVal = TRUE;
	u32 u32SN = 0xffffffff;			//�ϵ��ʼ�����к�
	sCanFrame sTxCan;
	u32 i,j;
	//�ظ�֡����
	if(u32SN == sRxCan->u32ID.ID.SN)
		return FALSE;
	else
		u32SN = sRxCan->u32ID.ID.SN;
	
	//�����ҪӦ����Ӧ��֡
	if(sRxCan->u32ID.ID.ACK == eACK)
	{
		sTxCan.u32ID.ID.ACK = eNOACK;
		sTxCan.u32ID.ID.FT = sRxCan->u32ID.ID.FT;
		sTxCan.u32ID.ID.RD = sRxCan->u32ID.ID.RD;
		sTxCan.u32ID.ID.RID = sRxCan->u32ID.ID.TID;
		sTxCan.u32ID.ID.SN = sRxCan->u32ID.ID.SN;
		sTxCan.u32ID.ID.SUB = sRxCan->u32ID.ID.SUB;
		sTxCan.u32ID.ID.SUM = sRxCan->u32ID.ID.SUM;
		sTxCan.u32ID.ID.TID = sRxCan->u32ID.ID.RID;
		
		sTxCan.u16DLC = sRxCan->u16DLC;
		
		for(i = 0; i < sTxCan.u16DLC; i++)
			sTxCan.u8Data[i] = sRxCan->u8Data[i];
		
		WLTxCan.tx_efid = sTxCan.u32ID.u32Id;
		WLTxCan.tx_ff = CAN_FF_EXTENDED;
		WLTxCan.tx_dlen = sTxCan.u16DLC;
		for(i = 0; i < sTxCan.u16DLC; i++)
		{
			WLTxCan.tx_data[i] = sRxCan->u8Data[i];
		}
		can_message_transmit(CAN0, &WLTxCan);
		//InsCanTrsQueue(&sTxCan, TRUE);
	}
    /*
	 * ������֡,��֡��=u32ID.ID.SUM
	 */
	if((sRxCan->u32ID.ID.SUB == 0) && (sRxCan->u32ID.ID.SUM > 0))
	{
		/*
		 * ��¼�յ�CAN֡����֡����
		 */
		u32CanToWlFrmNum = sRxCan->u32ID.ID.SUM;//���֡����֡��
		u32WlBusDataLen = 0;
		u32CurFrm = 0;
		/*
		 * װ���������������ڿ��д�����֡ID
		 */
		st_WlTrsTmp.u32ID.ID.ACK = eNOACK;
		st_WlTrsTmp.u32ID.ID.FT = FT_SC_TO_WL_DBUS;
		st_WlTrsTmp.u32ID.ID.RD = 0;
		st_WlTrsTmp.u32ID.ID.RID = 0;
		st_WlTrsTmp.u32ID.ID.SN = 0;
		st_WlTrsTmp.u32ID.ID.SUB = 0;
		st_WlTrsTmp.u32ID.ID.SUM = 0;
		st_WlTrsTmp.u32ID.ID.TID = 0;
		
		/*
		 * װ����������������ݵ�֡ID����
		 */
		for (i=0;i<4;i++)
		{
			st_WlTrsTmp.u8WlData[i] = (((u8)(st_WlTrsTmp.u32ID.u32Id >> (i << 3))) & 0xff);//29λ��չ֡ID��ռ��4���ֽ�
		}
		
		// ȡ��Ҫ���͵ĵ�ַ
		st_WlTrsTmp.u8Addr = sRxCan->u8Data[0];		// ȡ��֧�ܿ�����Ҫ���͵����ߵ�ַ��Ϊ���ߺ���ģ��ķ��͵�ַ
		
		st_WlTrsTmp.u8WlData[i] = u32WlBusDataLen; //���ݳ���
		
		for(j=0;j<sRxCan->u16DLC;j++)
		{
			st_WlTrsTmp.u8WlData[i + 1 + j] = sRxCan->u8Data[j];
		}
		u32WlBusDataLen += sRxCan->u16DLC;
		
		return FALSE;
	}
	/*
	 * ������֡,�����֮֡�䲻����������
	 */
	if(sRxCan->u32ID.ID.SUB == 1)
	{
		if(u32CurFrm < u32CanToWlFrmNum)
		{
			if(u32CurFrm == sRxCan->u32ID.ID.SUM)//Ӧ���յ���֡��źͽ��յ�����֡�����ͬ
			{
				for(i=0;i<sRxCan->u16DLC;i++)//��������
				{
					st_WlTrsTmp.u8WlData[u32WlBusDataLen + i + 5] = sRxCan->u8Data[i];
				}
				u32WlBusDataLen += sRxCan->u16DLC;
				u32CurFrm++;//��һ֡Ӧ���յ���֡���
			}
			if(u32CurFrm == u32CanToWlFrmNum)//���֡�������
			{
				/*
				 * ��������������ת����������510��ʵ��������ַ0xFF
				 */
				//st_WlTrsTmp.u8Addr = 0xFF;		// ��������ַ
				st_WlTrsTmp.u8Ack = 0;
				st_WlTrsTmp.u8FT = sRxCan->u32ID.ID.FT;
				st_WlTrsTmp.u32Len = u32WlBusDataLen+5;
				st_WlTrsTmp.u8WlData[4] = u32WlBusDataLen;
				
				//InsWlTxBuf(&st_WlTrs, TRUE);
				WL_SendData(st_WlTrsTmp.u8Addr,st_WlTrsTmp.u8WlData,st_WlTrsTmp.u32Len);
//				SetLedState(LED_CAN_TRANS,TRUE,3);
				return TRUE;
			}
			return FALSE;
		}
	}
	return FALSE;
}
/*******************************************************************************************

*******************************************************************************************/
//У׼X��Y�Ƕ�ֵ
extern u32 AngleSensorCalibrateXYFunction(LOGIC_SET_ANGLEPARAM_TYPE *pData);

void InitCanRxProc(sCAN_FRAME CanRX_Proc)
{
	OS_TCB pdata;
	u32 u32Sum;
	u32 i;
	
	CXB_CAN_FRAME_TYPE stFrame;
	
	sCanFrame IRCanRX_Proc;

	stFRAME SendFrame;
	u16 u16VauleTemp;
	LOGIC_SET_PARAMNUMB_TYPE sParamNumbTemp;
	
	CanHeadID tmp;
	tmp.u32Id = CanRX_Proc.Stdid;
	
	IRCanRX_Proc.u32ID.u32Id = CanRX_Proc.Stdid;
	IRCanRX_Proc.u16DLC = CanRX_Proc.DLC;
	for (i = 0; i < IRCanRX_Proc.u16DLC; i++)
	{
		IRCanRX_Proc.u8Data[i] = CanRX_Proc.Data[i];
	}
	
	if (tmp.ID.RD == RD_GLOBAL_FRAME)//����֡
	{
		//ʹ����Ա��λ����
		if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
		{
			if (tmp.ID.FT == SET_UWB_PARA && tmp.ID.RID == UWB_ID)		//������Ա��λģ�鹤������
			{
				memcpy(&gUwbRunPara, CanRX_Proc.Data, sizeof(gUwbRunPara));
				gUwbState = UWB_NORMAL;
				OSSemPost(gUwbStart);
			}
		}
		
	/**���͸��Ƕȴ�����������**/
	#if 1
	//ʹ�ܽǶȲɼ�����
	if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
	{
		if (tmp.ID.RID == ANGLE_ID)				//����֡
		{
			switch(tmp.ID.FT)
			{
				case ANGLE_CHECK_VALUE:				//�ǶȲ�ѯ֡
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break;
					
					STM32_CAN_Write(0, SendAngleData(ANGLE_CHECK_VALUE, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED); // ����Ӧ��
					u16CanLeftSendTimer = 0x00;
				}
				break;

				case ANGLE_SET_DEV_TYPE:			//�����豸����
				{
					if(CanRX_Proc.DLC != CAN_LENGTH_8)
						break;
			
					u32Sum = SYSTEM_RUN_STATUS_ADJUST;
					LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);

					if(LogicParamApi(LOGIC_SET_DEVTYPE_INF, &(CanRX_Proc.Data[0x00])))	
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_SET_DEV_TYPE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��	

					}
				}
				break;

				case ANGLE_GET_DEV_TYPE:			//��ȡ�豸����
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break ;			

					if(LogicParamApi(LOGIC_GET_DEVTYPE_INF, &(CanRX_Proc.Data[0x00])))	
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_GET_DEV_TYPE_ACK, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��
					}
					u16CanLeftSendTimer = 0x00;
				}
				break;
					
				case ANGLE_SET_MODIFY_PARAM_NUMB:			//���ó�������ֵ����
				{
					if(CanRX_Proc.DLC != CAN_LENGTH_6)
						break ;
					
					u32Sum = SYSTEM_RUN_STATUS_ADJUST;
					LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);

					if(LogicParamApi(LOGIC_SET_MODIFY_PARAM_NUMB, &(CanRX_Proc.Data[0x02])))	
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_SET_MODIFY_PARAM_NUMB, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��	
					}
				}
				break;
					
				case ANGLE_GET_MODIFY_PARAM_NUMB:			//�ض���������ֵ����
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break ;	
						
					if(LogicParamApi(LOGIC_GET_MODIFY_PARAM_NUMB, &(CanRX_Proc.Data[0x02])))	
					{				
						STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_PARAM_NUMB_ACK, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��	
					}
					u16CanLeftSendTimer = 0x00;
				}
				break;

				case ANGLE_SET_MODIFY_VALUE:			//���ó�������ֵ
				{
					if(CanRX_Proc.DLC != CAN_LENGTH_8)
						break ;
					
					u32Sum = SYSTEM_RUN_STATUS_ADJUST;
					LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);
					
					AngleSensorCalibrateXYFunction((LOGIC_SET_ANGLEPARAM_TYPE *)&(CanRX_Proc.Data[0x00]));
					if(LogicParamApi(LOGIC_SET_MODIFY_PARAM, &(CanRX_Proc.Data[0x00])))	
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_SET_MODIFY_VALUE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��	
					}
				}
				break;

				case ANGLE_GET_MODIFY_VALUE:			//�ض���������ֵ
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break ;

					LogicParamApi(LOGIC_GET_MODIFY_PARAM_NUMB, &sParamNumbTemp);	
					
					SendFrame.u8DT[0x01] = ANGLE_TYPE_X;
					for(u16VauleTemp = 0x00; u16VauleTemp < sParamNumbTemp.u16AngleParamNumbX; u16VauleTemp++)
					{
						SendFrame.u8DT[0x02] = 0x00;
						SendFrame.u8DT[0x03] = 0x00;
						SendFrame.u8DT[0x04] = 0x00;
						SendFrame.u8DT[0x05] = 0x00;	
						SendFrame.u8DT[0x06] = (u8)(u16VauleTemp & 0xFF);
						SendFrame.u8DT[0x07] = (u8)((u16VauleTemp & 0xFF00) >> 8);	
						if(LogicParamApi(LOGIC_GET_MODIFY_PARAM, &(SendFrame.u8DT[0x00])))
						{
							STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_VALUE_ACK, tmp.ID.TID, (uint8_t *)&SendFrame.u8DT, 0), CAN_FF_EXTENDED);		//����Ӧ��
						}
						OSTimeDly(10);
					}
					SendFrame.u8DT[0x01] = ANGLE_TYPE_Y;
					for(u16VauleTemp = 0x00; u16VauleTemp <sParamNumbTemp.u16AngleParamNumbY; u16VauleTemp++)
					{
						SendFrame.u8DT[0x02] = 0x00;
						SendFrame.u8DT[0x03] = 0x00;
						SendFrame.u8DT[0x04] = 0x00;
						SendFrame.u8DT[0x05] = 0x00;	
						SendFrame.u8DT[0x06] = (u8)(u16VauleTemp&0xFF);
						SendFrame.u8DT[0x07] = (u8)((u16VauleTemp&0xFF00) >> 8);	
						if(LogicParamApi(LOGIC_GET_MODIFY_PARAM, &(SendFrame.u8DT[0x00])))
						{
							STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_VALUE_ACK, tmp.ID.TID, (uint8_t *)&SendFrame.u8DT, 0), CAN_FF_EXTENDED);		//����Ӧ��
						}
						OSTimeDly(10);
					}	
					u16CanLeftSendTimer = 0x00;	
				}
				break;

				case ANGLE_MODIFY_PARAM_SAVE:			//��������ֵ��������
				{
					if(CanRX_Proc.DLC != CAN_LENGTH_2)
						break ;
					
					u32Sum = SYSTEM_RUN_STATUS_ADJUST;
					LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);
					
					if(CanRX_Proc.Data[0x01] != 0x01)
						break;
					if(LogicParamApi(LOGIC_SAVE_PARAM_MSG, NULL))
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_MODIFY_PARAM_SAVE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��	
						OSTimeDly(10);
						//ִ����ת
						IapJumpToBoot(IN_FLASH_BOOTLOADER_ADDR);
					}
				}
				break;

				case ANGLE_SET_WORK_PARAM:					//���ù�������
				{
					if(CanRX_Proc.DLC != CAN_LENGTH_8 && CanRX_Proc.Data[6] != ANGLE_SUB_TYPE)
						break ;
					
					u16VauleTemp = (u16)CanRX_Proc.Data[0x01];
					LogicRunInfApi(LOGIC_SET_WORK_MODE, &u16VauleTemp);	
					u16VauleTemp = (u16)CanRX_Proc.Data[0x02];
					u16VauleTemp |= (u16)(CanRX_Proc.Data[0x03] << 8);
					if(u16VauleTemp < CANLEFT_REPORT_TIME_MIN)
						u16VauleTemp = CANLEFT_REPORT_TIME_MIN;
					LogicRunInfApi(LOGIC_SET_REPORT_INTERVAL, &u16VauleTemp);
					
					u16CanLeftSendInterval = u16VauleTemp;
					
		//			if(CanRX_Proc.Data[0x04] < (DYK_DEV_TYPE_MAX & 0xFF))
		//			{
		//				u32LeftReportRxDevId = CanRX_Proc.Data[0x04];				
		//			}
						
					u16CanLeftSystemRunStatus = SYSTEM_RUN_STATUS_NORMAL;    //�ú���2020/03/09
					
		//			u8LeftActiveReportAngleChange = CanRX_Proc.Data[0x05];           //�ú���2020/03/09
					/*************************��can
					CanRightSetTemporaryStatus(u16CanLeftSystemRunStatus, u32LeftReportRxDevId, u8LeftActiveReportAngleChange);
					*****************************/
					
					STM32_CAN_Write(0, SendAngleData(ANGLE_SET_WORK_PARAM, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//����Ӧ��	
				}
				break;

				case ANGLE_GET_WORK_PARAM:			//�ض���������
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break ;

					STM32_CAN_Write(0, SendAngleData(ANGLE_GET_WORK_PARAM_ACK, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//����Ӧ��
					u16CanLeftSendTimer = 0x00;
				}
				break;

				case ANGLE_READ_VERSION:			//��ȡ�汾��
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break ;

					STM32_CAN_Write(0, SendAngleData(ANGLE_READ_VERSION_ACK, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//����Ӧ��
					u16CanLeftSendTimer = 0x00;	
				}
				break;

				default:
					break;
			}
		}
	}
	#endif
	
		/**���͸����ⱨ���������ݣ�����д���**/
		if ((CanRX_Proc.Stdid == FRM_ID_BEFORE_ALARM) && (CanRX_Proc.DLC == 0x05))		// ���ⱨ����Ԥ��֡ID
		{
			BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;					//�յ�Ԥ��Ԥ������峬ʱ��ʱ��
			Led_PowerOn();
			SetCurrentLightType(LED_CLASSIC_MODE);				//���ⱨ����������
			if ((CanRX_Proc.Data[0] == 0x01) && (CanRX_Proc.Data[1] == 0x01))			//��һ���ֽ�Ϊ0x01
			{
				BeepCaseFlag = CanRX_Proc.Data[0];							//��
				LightFlag = CanRX_Proc.Data[1];								//��
			}
			else if ((CanRX_Proc.Data[0] == AGING_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//��һ���ֽ������豸�ϻ�LEDʹ�ã�����4��LED�����ϻ�������
			{		//�豸����
				LightFlag = AGING_TEST_BYTE;				//�Ƴ���
				BeepCaseFlag = 0;							//������
			}
			else if ((CanRX_Proc.Data[0] == POWER_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//��һ���ֽ������豸���飬�����������ģ�����4��LED������������
			{
				LightFlag = POWER_TEST_BYTE;					//�Ƴ���
				BeepCaseFlag = 0xffff;							//������
			}
			else if ((CanRX_Proc.Data[0] == LED_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//��һ���ֽ������豸���飬����LED�Ƿ�������˳�������ɫLED������������
			{
				LightFlag = LED_TEST_BYTE;						//�Ƴ���
				BeepCaseFlag = 0;								//������
			}
			else
			{
				if((CanRX_Proc.Data[1] == 0x10)
				|| (CanRX_Proc.Data[1] == 0x11)
				|| (CanRX_Proc.Data[1] == 0x12)
				|| (CanRX_Proc.Data[1] == 0x21)
				|| (CanRX_Proc.Data[1] == 0x22)
				|| (CanRX_Proc.Data[1] == 0x23))    //���Ƴ���
				{
					LightFlag = CanRX_Proc.Data[1];           						//��
					u16BeepOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
					u16LightOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
					u16BeepOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
					u16LightOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
				}
				else
				{
					LightFlag = 0;
				}
				//jhy
				if((CanRX_Proc.Data[0] == 0x11)
				|| (CanRX_Proc.Data[0] == 0x01))    //���Ƴ���
				{
					BeepCaseFlag = CanRX_Proc.Data[0]; 
					u16BeepOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
					u16LightOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
					u16BeepOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
					u16LightOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
				}
				else
				{
					BeepCaseFlag = 0;
				}
			}
		}
		else if ((CanRX_Proc.Stdid == FRM_ID_ACTION_ALARM) && (CanRX_Proc.DLC == 0x05))		// ���ⱨ������������֡ID
		{
			BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;			//�յ�����Ԥ������峬ʱ��ʱ��
			Led_PowerOn();
			SetCurrentLightType(LED_CLASSIC_MODE);				//���ⱨ����������
			if ((CanRX_Proc.Data[0] == 0x01) 
			&& (CanRX_Proc.Data[1] == 0x02))			//��һ���ֽ�Ϊ0x01
			{
				BeepCaseFlag = CanRX_Proc.Data[0];
				LightFlag = CanRX_Proc.Data[1];
			}
			else if((CanRX_Proc.Data[0] == 0x01) 
			&& (CanRX_Proc.Data[1] == 0x03))
			{
				BeepCaseFlag = CanRX_Proc.Data[0];
				LightFlag = CanRX_Proc.Data[1];
			}
			else
			{
				//jhy
				if((CanRX_Proc.Data[1] == 0x10)
				|| (CanRX_Proc.Data[1] == 0x11)
				|| (CanRX_Proc.Data[1] == 0x12)
				|| (CanRX_Proc.Data[1] == 0x21)
				|| (CanRX_Proc.Data[1] == 0x22)
				|| (CanRX_Proc.Data[1] == 0x23))    //���Ƴ���
				{
					LightFlag = CanRX_Proc.Data[1];           						//��
					u16BeepOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
					u16LightOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
					u16BeepOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
					u16LightOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
				}
				else
				{
					LightFlag = 0;
				}
				//jhy
				if((CanRX_Proc.Data[0] == 0x11)
				|| (CanRX_Proc.Data[0] == 0x01))    //���Ƴ���
				{
					BeepCaseFlag = CanRX_Proc.Data[0]; 
					u16BeepOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
					u16LightOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
					u16BeepOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
					u16LightOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
				}
				else
				{
					BeepCaseFlag = 0;
				}
			}
		}
		else if ((CanRX_Proc.Stdid == FRM_ID_ACTION_ALARM) && (CanRX_Proc.DLC == 0x08))		// ���ⱨ������������֡ID
		{
			BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;			//�յ�����Ԥ��������ó�ʱ��ʱ��
			Led_PowerOn();
			if (CanRX_Proc.Data[0] == 0x01)			//��һ���ֽ�Ϊ0x01
			{
				SetCurrentLightType(LED_EXTEND_MODE);				//���ⱨ����������
				LightFlag = CanRX_Proc.Data[0];		//LED��������ģʽ
				RGrayScale = CanRX_Proc.Data[1];
				GGrayScale = CanRX_Proc.Data[2];
				BGrayScale = CanRX_Proc.Data[3];
				u16LightOnTimer = (u16)(CanRX_Proc.Data[4] * 10);
				u16LightOffTimer = (u16)(CanRX_Proc.Data[5] * 10);
				if(CanRX_Proc.Data[6] == 0)
				{
					BeepCaseFlag = 1;		//��׼�����߼�
				}
				else if ((CanRX_Proc.Data[6]) == 0xf0)
				{
					BeepCaseFlag = 0xffff;		//
				}
				else if ((CanRX_Proc.Data[6]) == 0x0f)
				{
					BeepCaseFlag = 0x00;		//
				}
				else
				{
					BeepCaseFlag = 0x11;		//�����÷����߼�
					u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
					u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
				}
			}
			else if(CanRX_Proc.Data[0] == 0x02)		//��ˮ��Ч��
			{
				SetCurrentLightType(LED_EXTEND_MODE);				//���ⱨ����������
				LightFlag = CanRX_Proc.Data[0];		//LED��������ģʽ
				RGrayScale = CanRX_Proc.Data[1];
				GGrayScale = CanRX_Proc.Data[2];
				BGrayScale = CanRX_Proc.Data[3];
				LED_NUM = ((CanRX_Proc.Data[4] & 0xf0) >> 4);
				WaterFlow_Type = CanRX_Proc.Data[4] & 0x0f;
				u16LightOnTimer = (u16)(CanRX_Proc.Data[5] * 10);
				if(CanRX_Proc.Data[6] == 0)
				{
					BeepCaseFlag = 1;		//��׼�����߼�
				}
				else if ((CanRX_Proc.Data[6]) == 0xf0)
				{
					BeepCaseFlag = 0xffff;		//
				}
				else if ((CanRX_Proc.Data[6]) == 0x0f)
				{
					BeepCaseFlag = 0x00;		//
				}
				else
				{
					BeepCaseFlag = 0x11;		//�����÷����߼�
					u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
					u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
				}
			}
			else if(CanRX_Proc.Data[0] == 0x03)		//������Ч��
			{
				SetCurrentLightType(LED_EXTEND_MODE);				//���ⱨ����������
				LightFlag = CanRX_Proc.Data[0];		//LED��������ģʽ
				RGrayScale = CanRX_Proc.Data[1];
				GGrayScale = CanRX_Proc.Data[2];
				BGrayScale = CanRX_Proc.Data[3];

				if (RGrayScale != 0)
				{
					Breathestep_R = (RGrayScale >> 1) * CanRX_Proc.Data[4];
				}
				if (GGrayScale != 0)
				{
					Breathestep_G = (GGrayScale >> 1) * CanRX_Proc.Data[4];
				}
				if (BGrayScale != 0)
				{
					Breathestep_B = (BGrayScale >> 1) * CanRX_Proc.Data[4];
				}
				
				if(CanRX_Proc.Data[6] == 0)
				{
					BeepCaseFlag = 1;		//��׼�����߼�
				}
				else if ((CanRX_Proc.Data[6]) == 0xf0)
				{
					BeepCaseFlag = 0xffff;		//
				}
				else if ((CanRX_Proc.Data[6]) == 0x0f)
				{
					BeepCaseFlag = 0x00;		//
				}
				else
				{
					BeepCaseFlag = 0x11;		//�����÷����߼�
					u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
					u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
				}
			}
			else if(CanRX_Proc.Data[0] == 0x04)			//���ǵ�Ч��
			{
				SetCurrentLightType(LED_EXTEND_MODE);				//���ⱨ����������
				LightFlag = CanRX_Proc.Data[0];		//LED��������ģʽ
				RGrayScale = CanRX_Proc.Data[1];
				GGrayScale = CanRX_Proc.Data[2];
				BGrayScale = CanRX_Proc.Data[3];
				LED_NUM = CanRX_Proc.Data[4];
				u16LightOnTimer = (u16)(CanRX_Proc.Data[5] * 10);
				if(CanRX_Proc.Data[6] == 0)
				{
					BeepCaseFlag = 1;		//��׼�����߼�
				}
				else if ((CanRX_Proc.Data[6]) == 0xf0)
				{
					BeepCaseFlag = 0xffff;		//
				}
				else if ((CanRX_Proc.Data[6]) == 0x0f)
				{
					BeepCaseFlag = 0x00;		//
				}
				else
				{
					BeepCaseFlag = 0x11;		//�����÷����߼�
					u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
					u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
				}
			}
			else if(CanRX_Proc.Data[0] == 0x05)			//��ʵ�Ч��
			{
				SetCurrentLightType(LED_EXTEND_MODE);				//���ⱨ����������
				LightFlag = CanRX_Proc.Data[0];		//LED��������ģʽ
				RGrayScale = CanRX_Proc.Data[1];
				GGrayScale = CanRX_Proc.Data[1];
				BGrayScale = CanRX_Proc.Data[1];		//ֻʹ��ͬһ�ֻҶ���ֵ���ڻ�ɫʱ��ֵ��1:1���������������ɫ������
				LED_NUM = (CanRX_Proc.Data[2] & 0x3f);
				u16LightOnTimer = (u16)(CanRX_Proc.Data[3] * 10);
				if(CanRX_Proc.Data[6] == 0)
				{
					BeepCaseFlag = 1;		//��׼�����߼�
				}
				else if ((CanRX_Proc.Data[6]) == 0xf0)
				{
					BeepCaseFlag = 0xffff;		//
				}
				else if ((CanRX_Proc.Data[6]) == 0x0f)
				{
					BeepCaseFlag = 0x00;		//
				}
				else
				{
					BeepCaseFlag = 0x11;		//�����÷����߼�
					u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
					u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
				}
			}
			else
			{
				;
			}
		}
		
	/**���͸�������յ�����**/
	#if 1
	if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
	{
		if (tmp.ID.RID == WL_RID)				//����֡
		{
			switch (tmp.ID.FT)
			{
				case FT_SC_TO_WL_DBUS:							//2				// SC ���� WL ���������ݣ���ҪӦ��
					CanRcvBusProc(&IRCanRX_Proc);
				break;
				case FT_WL_TO_SC_DBUS:							//3				// WL ���� SC ���������ݣ���ҪӦ��
					
				break;
				case FT_SC_TO_WL_IR_MATCH_RST:					//5				// SC ���� WL �ĺ������������ҪӦ��
					CanRcvIrMatchProc(&IRCanRX_Proc);
				break;
				
				case FT_SC_TO_WL_RF_MATCH_RST:					//7				// SC ���� WL �����߶���������ҪӦ��
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
				break;
				case FT_SC_TO_WL_MATCH_SC_GROUP_INFO:			//8				// SC���͸�WL������ܷ��ͳ�����Ϣ	
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
				break;
				case FT_SC_TO_WL_UC_SC_GROUP_INFO:				//9				// SC���͸�WL�����ؼܷ��ͳ�����Ϣ
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
				break;
				case FT_SC_TO_WL_CTL_DATA_RECEPT:				//11			// SC ���͸� ң�����Ŀ��������Ƿ���գ���ҪӦ��			
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
				break;
				case FT_SC_TO_WL_DISCONNECT:					//14			// SC ���͸� WL �������
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
				break;
				case FT_SC_TO_WL_RESET_PAR:						//16			// SC ���͸� WL �������
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
				break;
				case FT_SC_TO_WL_LIFT_RECEPT:					//17			// SC ���͸� WL ����̧���Ƿ����
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
				break;
				
				case FT_WL_TO_SC_AUTO_PRESS_CLOSE:					//20			// SC ���͸� WL ����̧���Ƿ����
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[0]);
				break;				
				case FT_SC_TO_WL_RESET_PAR_WL:					//22			// �������ߺ������ģ�����
					CanRcvSetPar(&IRCanRX_Proc);						//PT_SER_SC_NO;
				break;
//				case FT_SC_TO_WL_MATCH_END:						//31			// ����ֱ�ӽ���
	//				SetLedState(LED_GREEN,FALSE,0x00);
	//				SetLedState(LED_RED,FALSE,0x00);
//				break;
				
				default:
					break;
			}
		}
	}
	#endif
	}
	else if (tmp.ID.RD == RD_PRG_FRAME)			//�����������֡
	{
		u32CanLeftPrgRecvTimer = CANLEFT_PRG_SEND_TIMEOUT;

		stFrame.u32ID.u32Id = tmp.u32Id;
		stFrame.u16DLC = CanRX_Proc.DLC;
		memcpy(stFrame.u8DT, CanRX_Proc.Data, stFrame.u16DLC);

		if((stFrame.u32ID.ID.PACKET_NUMB) == 0x01)			//�յ�������µ�һ��
		{	
			if((stFrame.u32ID.ID.TD_FTYPE) == CXB_FRAME_DOWNLOAD_PRG_VERSION)
			{
				u32Sum = SYSTEM_RUN_STATUS_UPPRG;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);				
				//ʹ�ܽǶȲɼ�����
				if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
				{
					/**���𲻱�Ҫ������**/
					OSTaskQuery(ANGLE_SENSOR_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(ANGLE_SENSOR_TASK_PRIO);
					}
				}
				//ʹ����Ա��λ����
				if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
				{
					/**���𲻱�Ҫ������**/
					OSTaskQuery(UWBAPP_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{
						OSTaskSuspend(UWBAPP_TASK_PRIO);
					}
					/**���𲻱�Ҫ������**/
					OSTaskQuery(CANTX_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(CANTX_TASK_PRIO);
					}
				}
				//ʹ��433����ģ�鹦��
				if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
				{
					/**���𲻱�Ҫ������**/
					OSTaskQuery(WL_MANAGE_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(WL_MANAGE_TASK_PRIO);
					}
				}
			}
		}
		//�յ�����ĳ���֡ʱ�Ĵ���
		CanRecvProgProc(&stFrame);
	}
	else
	{
		return;
	}

}
/*******************************************************************************************

*******************************************************************************************/
__weak void CanRxParaProcess(void)
{
	;
}


void NormalCanRxProc(sCAN_FRAME CanRX_Proc)
{
	OS_TCB pdata;
	CXB_CAN_FRAME_TYPE stFrame;
	CanHeadID tmp;
	
	stFRAME SendFrame;
	u16 u16VauleTemp;
	u32 u32Sum;
	LOGIC_SET_PARAMNUMB_TYPE sParamNumbTemp;
	
	tmp.u32Id = CanRX_Proc.Stdid;
	
	u32 i;
	
	sCanFrame IRCanRX_Proc;

	tmp.u32Id = CanRX_Proc.Stdid;
	
	IRCanRX_Proc.u32ID.u32Id = CanRX_Proc.Stdid;
	IRCanRX_Proc.u16DLC = CanRX_Proc.DLC;
	for (i = 0; i < IRCanRX_Proc.u16DLC; i++)
	{
		IRCanRX_Proc.u8Data[i] = CanRX_Proc.Data[i];
	}
	
	
	if (tmp.ID.RD == RD_GLOBAL_FRAME)				//����֡
	{
		//ʹ����Ա��λ����
		if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
		{
			if (tmp.ID.RID == UWB_ID)
			{
				switch (tmp.ID.FT)
				{
					case REQ_UWB_INFO:			//2����ѯ��Ա��λģ��ı�ǩ����
						break;

					case SET_UWB_PARA:
						if(tmp.ID.RID == UWB_ID)
						{
							memcpy(&gUwbRunPara, CanRX_Proc.Data, sizeof(gUwbRunPara));
							CanRxParaProcess();
						}
						break;

					case UWB_RESET:		//6����λ��Ա��λģ������ⱨ����
						if(tmp.ID.RID == UWB_ID)
						{
							NVIC_SystemReset();
						}
						
					default:
						break;
				}
			}
		}
	
/**���͸��Ƕȴ�����������**/
//#if 0
	/**���͸��Ƕȴ����������ݣ�����д���**/
	if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
	{
		if (tmp.ID.RID == ANGLE_ID)
		{
		switch(tmp.ID.FT)
		{
			case ANGLE_CHECK_VALUE:				//�ǶȲ�ѯ֡
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break;
				
				STM32_CAN_Write(0, SendAngleData(ANGLE_CHECK_VALUE, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED); // ����Ӧ��
				u16CanLeftSendTimer = 0x00;
			}
			break;

			case ANGLE_SET_DEV_TYPE:			//�����豸����
			{
				if(CanRX_Proc.DLC != CAN_LENGTH_8)
					break;
		
				u32Sum = SYSTEM_RUN_STATUS_ADJUST;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);

				if(LogicParamApi(LOGIC_SET_DEVTYPE_INF, &(CanRX_Proc.Data[0x00])))	
				{
					STM32_CAN_Write(0, SendAngleData(ANGLE_SET_DEV_TYPE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��	

				}
			}
			break;

			case ANGLE_GET_DEV_TYPE:			//��ȡ�豸����
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break ;			

				if(LogicParamApi(LOGIC_GET_DEVTYPE_INF, &(CanRX_Proc.Data[0x00])))	
				{
					STM32_CAN_Write(0, SendAngleData(ANGLE_GET_DEV_TYPE_ACK, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��
				}
				u16CanLeftSendTimer = 0x00;
			}
			break;
				
			case ANGLE_SET_MODIFY_PARAM_NUMB:			//���ó�������ֵ����
			{
				if(CanRX_Proc.DLC != CAN_LENGTH_6)
					break ;
				
				u32Sum = SYSTEM_RUN_STATUS_ADJUST;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);

				if(LogicParamApi(LOGIC_SET_MODIFY_PARAM_NUMB, &(CanRX_Proc.Data[0x02])))	
				{
					STM32_CAN_Write(0, SendAngleData(ANGLE_SET_MODIFY_PARAM_NUMB, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��	
				}
			}
			break;
				
			case ANGLE_GET_MODIFY_PARAM_NUMB:			//�ض���������ֵ����
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break ;	
					
				if(LogicParamApi(LOGIC_GET_MODIFY_PARAM_NUMB, &(CanRX_Proc.Data[0x02])))	
				{				
					STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_PARAM_NUMB_ACK, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��	
				}
				u16CanLeftSendTimer = 0x00;
			}
			break;

			case ANGLE_SET_MODIFY_VALUE:			//���ó�������ֵ
			{
				if(CanRX_Proc.DLC != CAN_LENGTH_8)
					break ;
				
				u32Sum = SYSTEM_RUN_STATUS_ADJUST;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);
				
				AngleSensorCalibrateXYFunction((LOGIC_SET_ANGLEPARAM_TYPE *)&(CanRX_Proc.Data[0x00]));
				if(LogicParamApi(LOGIC_SET_MODIFY_PARAM, &(CanRX_Proc.Data[0x00])))	
				{
					STM32_CAN_Write(0, SendAngleData(ANGLE_SET_MODIFY_VALUE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��	
				}
			}
			break;

			case ANGLE_GET_MODIFY_VALUE:			//�ض���������ֵ
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break ;

				LogicParamApi(LOGIC_GET_MODIFY_PARAM_NUMB, &sParamNumbTemp);	
				
				SendFrame.u8DT[0x01] = ANGLE_TYPE_X;
				for(u16VauleTemp = 0x00; u16VauleTemp < sParamNumbTemp.u16AngleParamNumbX; u16VauleTemp++)
				{
					SendFrame.u8DT[0x02] = 0x00;
					SendFrame.u8DT[0x03] = 0x00;
					SendFrame.u8DT[0x04] = 0x00;
					SendFrame.u8DT[0x05] = 0x00;	
					SendFrame.u8DT[0x06] = (u8)(u16VauleTemp & 0xFF);
					SendFrame.u8DT[0x07] = (u8)((u16VauleTemp & 0xFF00) >> 8);	
					if(LogicParamApi(LOGIC_GET_MODIFY_PARAM, &(SendFrame.u8DT[0x00])))
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_VALUE_ACK, tmp.ID.TID, (uint8_t *)&SendFrame.u8DT, 0), CAN_FF_EXTENDED);		//����Ӧ��
					}
					OSTimeDly(10);
				}
				SendFrame.u8DT[0x01] = ANGLE_TYPE_Y;
				for(u16VauleTemp = 0x00; u16VauleTemp <sParamNumbTemp.u16AngleParamNumbY; u16VauleTemp++)
				{
					SendFrame.u8DT[0x02] = 0x00;
					SendFrame.u8DT[0x03] = 0x00;
					SendFrame.u8DT[0x04] = 0x00;
					SendFrame.u8DT[0x05] = 0x00;	
					SendFrame.u8DT[0x06] = (u8)(u16VauleTemp & 0xFF);
					SendFrame.u8DT[0x07] = (u8)((u16VauleTemp & 0xFF00) >> 8);	
					if(LogicParamApi(LOGIC_GET_MODIFY_PARAM, &(SendFrame.u8DT[0x00])))
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_VALUE_ACK, tmp.ID.TID, (uint8_t *)&SendFrame.u8DT, 0), CAN_FF_EXTENDED);		//����Ӧ��
					}
					OSTimeDly(10);
				}
				u16CanLeftSendTimer = 0x00;	
			}
			break;

			case ANGLE_MODIFY_PARAM_SAVE:			//��������ֵ��������
			{
				if(CanRX_Proc.DLC != CAN_LENGTH_2)
					break ;
				
				u32Sum = SYSTEM_RUN_STATUS_ADJUST;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);
				
				if(CanRX_Proc.Data[0x01] != 0x01)
					break;
				if(LogicParamApi(LOGIC_SAVE_PARAM_MSG, NULL))
				{
					STM32_CAN_Write(0, SendAngleData(ANGLE_MODIFY_PARAM_SAVE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//����Ӧ��	
					OSTimeDly(10);
					//ִ����ת
					IapJumpToBoot(IN_FLASH_BOOTLOADER_ADDR);			//��λ
				}
			}
			break;

			case ANGLE_SET_WORK_PARAM:					//���ù�������
			{
				if(CanRX_Proc.DLC != CAN_LENGTH_8 && CanRX_Proc.Data[6] != ANGLE_SUB_TYPE)
					break ;
				
				u16VauleTemp = (u16)CanRX_Proc.Data[0x01];
				LogicRunInfApi(LOGIC_SET_WORK_MODE, &u16VauleTemp);	
				u16VauleTemp = (u16)CanRX_Proc.Data[0x02];
				u16VauleTemp |= (u16)(CanRX_Proc.Data[0x03]<<8);
				if(u16VauleTemp < CANLEFT_REPORT_TIME_MIN)
					u16VauleTemp = CANLEFT_REPORT_TIME_MIN;
				LogicRunInfApi(LOGIC_SET_REPORT_INTERVAL, &u16VauleTemp);
				
				u16CanLeftSendInterval = u16VauleTemp;
				
	//			if(CanRX_Proc.Data[0x04] < (DYK_DEV_TYPE_MAX & 0xFF))
	//			{
	//				u32LeftReportRxDevId = CanRX_Proc.Data[0x04];				
	//			}
					
				u16CanLeftSystemRunStatus = SYSTEM_RUN_STATUS_NORMAL;    //�ú���2020/03/09
				
	//			u8LeftActiveReportAngleChange = CanRX_Proc.Data[0x05];           //�ú���2020/03/09
				/*************************��can
				CanRightSetTemporaryStatus(u16CanLeftSystemRunStatus, u32LeftReportRxDevId, u8LeftActiveReportAngleChange);
				*****************************/
				
				STM32_CAN_Write(0, SendAngleData(ANGLE_SET_WORK_PARAM, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//����Ӧ��	
			}
			break;

			case ANGLE_GET_WORK_PARAM:			//�ض���������
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break;

				STM32_CAN_Write(0, SendAngleData(ANGLE_GET_WORK_PARAM_ACK, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//����Ӧ��
				u16CanLeftSendTimer = 0x00;
			}
			break;

			case ANGLE_READ_VERSION:			//��ȡ�汾��
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break;

				STM32_CAN_Write(0, SendAngleData(ANGLE_READ_VERSION_ACK, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//����Ӧ��
				u16CanLeftSendTimer = 0x00;	
			}
			break;

			default:
				break;
		}
		}
	}


//#endif
	
	/**���͸����ⱨ���������ݣ�����д���**/
	if ((CanRX_Proc.Stdid == FRM_ID_BEFORE_ALARM) && (CanRX_Proc.DLC == 0x05))		// ���ⱨ����Ԥ��֡ID
	{
		BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;					//�յ�Ԥ��Ԥ������峬ʱ��ʱ��
		Led_PowerOn();
		SetCurrentLightType(LED_CLASSIC_MODE);				//���ⱨ����������
		if ((CanRX_Proc.Data[0] == 0x01) && (CanRX_Proc.Data[1] == 0x01))			//��һ���ֽ�Ϊ0x01
		{
			BeepCaseFlag = CanRX_Proc.Data[0];							//��
			LightFlag = CanRX_Proc.Data[1];								//��
		}
		else if ((CanRX_Proc.Data[0] == AGING_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//��һ���ֽ������豸�ϻ�LEDʹ�ã�����4��LED�����ϻ�������
		{		//�豸����
			LightFlag = AGING_TEST_BYTE;				//�Ƴ���
			BeepCaseFlag = 0;							//������
		}
		else if ((CanRX_Proc.Data[0] == POWER_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//��һ���ֽ������豸���飬�����������ģ�����4��LED������������
		{
			LightFlag = POWER_TEST_BYTE;					//�Ƴ���
			BeepCaseFlag = 0xffff;							//������
		}
		else if ((CanRX_Proc.Data[0] == LED_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//��һ���ֽ������豸���飬����LED�Ƿ�������˳�������ɫLED������������
		{
			LightFlag = LED_TEST_BYTE;						//�Ƴ���
			BeepCaseFlag = 0;							//������
		}
		else
		{
			if ((CanRX_Proc.Data[1] == 0x10)
			|| (CanRX_Proc.Data[1] == 0x11)
			|| (CanRX_Proc.Data[1] == 0x12)
			|| (CanRX_Proc.Data[1] == 0x21)
			|| (CanRX_Proc.Data[1] == 0x22)
			|| (CanRX_Proc.Data[1] == 0x23))    //���Ƴ���
			{
				LightFlag = CanRX_Proc.Data[1];           						//��
				u16BeepOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
				u16LightOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
				u16BeepOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
				u16LightOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
			}
			else
			{
				LightFlag = 0;
			}
			//jhy
			if((CanRX_Proc.Data[0] == 0x11)
			|| (CanRX_Proc.Data[0] == 0x01))    //���Ƴ���
			{
				BeepCaseFlag = CanRX_Proc.Data[0]; 
				u16BeepOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
				u16LightOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
				u16BeepOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
				u16LightOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
			}
			else
			{
				BeepCaseFlag = 0;
			}
		}
	}
	else if ((CanRX_Proc.Stdid == FRM_ID_ACTION_ALARM) && (CanRX_Proc.DLC == 0x05))		// ���ⱨ������������֡ID
	{
		BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;			//�յ�����Ԥ������峬ʱ��ʱ��
		Led_PowerOn();
		SetCurrentLightType(LED_CLASSIC_MODE);				//���ⱨ����������
		if ((CanRX_Proc.Data[0] == 0x01) 
		&& (CanRX_Proc.Data[1] == 0x02))			//��һ���ֽ�Ϊ0x01
		{
			BeepCaseFlag = CanRX_Proc.Data[0];
			LightFlag = CanRX_Proc.Data[1];
		}
		else if((CanRX_Proc.Data[0] == 0x01) 
		&& (CanRX_Proc.Data[1] == 0x03))
		{
			BeepCaseFlag = CanRX_Proc.Data[0];
			LightFlag = CanRX_Proc.Data[1];
		}
		else
		{
			//jhy
			if((CanRX_Proc.Data[1] == 0x10)
			|| (CanRX_Proc.Data[1] == 0x11)
			|| (CanRX_Proc.Data[1] == 0x12)
			|| (CanRX_Proc.Data[1] == 0x21)
			|| (CanRX_Proc.Data[1] == 0x22)
			|| (CanRX_Proc.Data[1] == 0x23))    //���Ƴ���
			{
				LightFlag = CanRX_Proc.Data[1];           						//��
				u16BeepOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
				u16LightOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
				u16BeepOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
				u16LightOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
			}
			else
			{
				LightFlag = 0;
			}
			//jhy
			if((CanRX_Proc.Data[0] == 0x11)
			|| (CanRX_Proc.Data[0] == 0x01))    //���Ƴ���
			{
				BeepCaseFlag = CanRX_Proc.Data[0]; 
				u16BeepOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
				u16LightOnTimer = (u16)(CanRX_Proc.Data[2] * 10);
				u16BeepOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
				u16LightOffTimer = (u16)(CanRX_Proc.Data[3] * 10);
			}
			else
			{
				BeepCaseFlag = 0;
			}
		}
	}
	else if ((CanRX_Proc.Stdid == FRM_ID_ACTION_ALARM) && (CanRX_Proc.DLC == 0x08))		// ���ⱨ������������֡ID
	{
		BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;			//�յ�����Ԥ��������ó�ʱ��ʱ��
		Led_PowerOn();
		if (CanRX_Proc.Data[0] == 0x01)			//��һ���ֽ�Ϊ0x01
		{
			SetCurrentLightType(LED_EXTEND_MODE);				//���ⱨ����������
			LightFlag = CanRX_Proc.Data[0];		//LED��������ģʽ
			RGrayScale = CanRX_Proc.Data[1];
			GGrayScale = CanRX_Proc.Data[2];
			BGrayScale = CanRX_Proc.Data[3];
			u16LightOnTimer = (u16)(CanRX_Proc.Data[4] * 10);
			u16LightOffTimer = (u16)(CanRX_Proc.Data[5] * 10);
			if(CanRX_Proc.Data[6] == 0)
			{
				BeepCaseFlag = 1;		//��׼�����߼�
			}
			else if ((CanRX_Proc.Data[6]) == 0xf0)
			{
				BeepCaseFlag = 0xffff;		//
			}
			else if ((CanRX_Proc.Data[6]) == 0x0f)
			{
				BeepCaseFlag = 0x00;		//
			}
			else
			{
				BeepCaseFlag = 0x11;		//�����÷����߼�
				u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
				u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
			}
		}
		else if(CanRX_Proc.Data[0] == 0x02)		//��ˮ��Ч��
		{
			SetCurrentLightType(LED_EXTEND_MODE);				//���ⱨ����������
			LightFlag = CanRX_Proc.Data[0];		//LED��������ģʽ
			RGrayScale = CanRX_Proc.Data[1];
			GGrayScale = CanRX_Proc.Data[2];
			BGrayScale = CanRX_Proc.Data[3];
			LED_NUM = ((CanRX_Proc.Data[4] & 0xf0) >> 4);
			WaterFlow_Type = CanRX_Proc.Data[4] & 0x0f;
			u16LightOnTimer = (u16)(CanRX_Proc.Data[5] * 10);
			if(CanRX_Proc.Data[6] == 0)
			{
				BeepCaseFlag = 1;		//��׼�����߼�
			}
			else if ((CanRX_Proc.Data[6]) == 0xf0)
			{
				BeepCaseFlag = 0xffff;		//
			}
			else if ((CanRX_Proc.Data[6]) == 0x0f)
			{
				BeepCaseFlag = 0x00;		//
			}
			else
			{
				BeepCaseFlag = 0x11;		//�����÷����߼�
				u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
				u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
			}
		}
		else if(CanRX_Proc.Data[0] == 0x03)		//������Ч��
		{
			SetCurrentLightType(LED_EXTEND_MODE);				//���ⱨ����������
			LightFlag = CanRX_Proc.Data[0];		//LED��������ģʽ
			RGrayScale = CanRX_Proc.Data[1];
			GGrayScale = CanRX_Proc.Data[2];
			BGrayScale = CanRX_Proc.Data[3];

			if (RGrayScale != 0)
			{
				Breathestep_R = (RGrayScale >> 1) * CanRX_Proc.Data[4];
			}
			if (GGrayScale != 0)
			{
				Breathestep_G = (GGrayScale >> 1) * CanRX_Proc.Data[4];
			}
			if (BGrayScale != 0)
			{
				Breathestep_B = (BGrayScale >> 1) * CanRX_Proc.Data[4];
			}
			
			if(CanRX_Proc.Data[6] == 0)
			{
				BeepCaseFlag = 1;		//��׼�����߼�
			}
			else if ((CanRX_Proc.Data[6]) == 0xf0)
			{
				BeepCaseFlag = 0xffff;		//
			}
			else if ((CanRX_Proc.Data[6]) == 0x0f)
			{
				BeepCaseFlag = 0x00;		//
			}
			else
			{
				BeepCaseFlag = 0x11;		//�����÷����߼�
				u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
				u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
			}
		}
		else if(CanRX_Proc.Data[0] == 0x04)			//���ǵ�Ч��
		{
			SetCurrentLightType(LED_EXTEND_MODE);				//���ⱨ����������
			LightFlag = CanRX_Proc.Data[0];		//LED��������ģʽ
			RGrayScale = CanRX_Proc.Data[1];
			GGrayScale = CanRX_Proc.Data[2];
			BGrayScale = CanRX_Proc.Data[3];
			LED_NUM = CanRX_Proc.Data[4];
			u16LightOnTimer = (u16)(CanRX_Proc.Data[5] * 10);
			if(CanRX_Proc.Data[6] == 0)
			{
				BeepCaseFlag = 1;		//��׼�����߼�
			}
			else if ((CanRX_Proc.Data[6]) == 0xf0)
			{
				BeepCaseFlag = 0xffff;		//
			}
			else if ((CanRX_Proc.Data[6]) == 0x0f)
			{
				BeepCaseFlag = 0x00;		//
			}
			else
			{
				BeepCaseFlag = 0x11;		//�����÷����߼�
				u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
				u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
			}
		}
		else if(CanRX_Proc.Data[0] == 0x05)			//��ʵ�Ч��
		{
			SetCurrentLightType(LED_EXTEND_MODE);				//���ⱨ����������
			LightFlag = CanRX_Proc.Data[0];		//LED��������ģʽ
			RGrayScale = CanRX_Proc.Data[1];
			GGrayScale = CanRX_Proc.Data[1];
			BGrayScale = CanRX_Proc.Data[1];
			LED_NUM = (CanRX_Proc.Data[2] & 0x3f);
			u16LightOnTimer = (u16)(CanRX_Proc.Data[3] * 10);
			if(CanRX_Proc.Data[6] == 0)
			{
				BeepCaseFlag = 1;		//��׼�����߼�
			}
			else if ((CanRX_Proc.Data[6]) == 0xf0)
			{
				BeepCaseFlag = 0xffff;		//
			}
			else if ((CanRX_Proc.Data[6]) == 0x0f)
			{
				BeepCaseFlag = 0x00;		//
			}
			else
			{
				BeepCaseFlag = 0x11;		//�����÷����߼�
				u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
				u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
			}
		}
		
		
		else
		{
			;
		}
	}
		/**���͸�������յ�����**/
		#if 1
		//ʹ��433����ģ�鹦��
		if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
		{
			if (tmp.ID.RID == WL_RID)				//����֡
			{
				switch (tmp.ID.FT)
				{
					case FT_SC_TO_WL_DBUS:							//2				// SC ���� WL ���������ݣ���ҪӦ��
						CanRcvBusProc(&IRCanRX_Proc);
					break;
					case FT_WL_TO_SC_DBUS:							//3				// WL ���� SC ���������ݣ���ҪӦ��
						
					break;
					case FT_SC_TO_WL_IR_MATCH_RST:					//5				// SC ���� WL �ĺ������������ҪӦ��
						CanRcvIrMatchProc(&IRCanRX_Proc);
					break;
					
					case FT_SC_TO_WL_RF_MATCH_RST:					//7				// SC ���� WL �����߶���������ҪӦ��
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
					break;
					case FT_SC_TO_WL_MATCH_SC_GROUP_INFO:			//8				// SC���͸�WL������ܷ��ͳ�����Ϣ	
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
					break;
					case FT_SC_TO_WL_UC_SC_GROUP_INFO:				//9				// SC���͸�WL�����ؼܷ��ͳ�����Ϣ
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
					break;
					case FT_SC_TO_WL_CTL_DATA_RECEPT:				//11			// SC ���͸� ң�����Ŀ��������Ƿ���գ���ҪӦ��			
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
					break;
					case FT_SC_TO_WL_DISCONNECT:					//14			// SC ���͸� WL �������
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
					break;
					case FT_SC_TO_WL_RESET_PAR:						//16			// SC ���͸� WL �������
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
					break;
					case FT_SC_TO_WL_LIFT_RECEPT:					//17			// SC ���͸� WL ����̧���Ƿ����
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
					break;
					case FT_SC_TO_WL_RESET_PAR_WL:					//22			// �������ߺ������ģ�����
						CanRcvSetPar(&IRCanRX_Proc);						//PT_SER_SC_NO;
					break;
				    case FT_WL_TO_SC_AUTO_PRESS_CLOSE:					//20			// SC ���͸� WL ����̧���Ƿ����
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[0]);
					break;	
//					case FT_SC_TO_WL_MATCH_END:						//31			// ����ֱ�ӽ���
//		//				SetLedState(LED_GREEN,FALSE,0x00);
//		//				SetLedState(LED_RED,FALSE,0x00);
//					break;
					
					default:
						break;
				}
			}
		}
		#endif
	}
	else if (tmp.ID.RD == RD_PRG_FRAME)					//�������֡
	{
		u32CanLeftPrgRecvTimer = CANLEFT_PRG_SEND_TIMEOUT;

		stFrame.u32ID.u32Id = tmp.u32Id;
		stFrame.u16DLC = CanRX_Proc.DLC;
		memcpy(stFrame.u8DT, CanRX_Proc.Data, stFrame.u16DLC);

		if((stFrame.u32ID.ID.PACKET_NUMB) == 0x01)
		{	
			if((stFrame.u32ID.ID.TD_FTYPE) == CXB_FRAME_DOWNLOAD_PRG_VERSION)
			{
				u32Sum = SYSTEM_RUN_STATUS_UPPRG;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);				
				//ʹ�ܽǶȹ���
				if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
				{
					/**���𲻱�Ҫ������**/
					OSTaskQuery(ANGLE_SENSOR_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(ANGLE_SENSOR_TASK_PRIO);
					}
				}
				//ʹ��UWB��Ա��λ����
				if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
				{
					/**���𲻱�Ҫ������**/
					OSTaskQuery(UWBAPP_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{
						OSTaskSuspend(UWBAPP_TASK_PRIO);
					}
					/**���𲻱�Ҫ������**/
					OSTaskQuery(CANTX_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(CANTX_TASK_PRIO);
					}
				}
				//ʹ��433����ģ�鹦��
				if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
				{
					/**���𲻱�Ҫ������**/
					OSTaskQuery(WL_MANAGE_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(WL_MANAGE_TASK_PRIO);
					}
				}
			}
		}
		//�յ�����ĳ���֡ʱ�Ĵ���
		CanRecvProgProc(&stFrame);
	}
	else
	{
		return;
	}
}

/*******************************************************************************************
**�������ƣ�CanRxProc
**�䡡�룺None
** �䡡����None
** ����������Can��������ʱ�������ݴ���
*******************************************************************************************/
void CanRxProc(sCAN_FRAME CanRX_Proc)
{
	switch(gUwbState)
	{
		case UWB_INIT:
			InitCanRxProc(CanRX_Proc);
		break;

		case UWB_NORMAL:
			NormalCanRxProc(CanRX_Proc);
		break;

		default:
		break;
	}
}
/*******************************************************************************************
**�������ƣ�void CanRx_Task(void *p_arg)
**�䡡�룺None
** �䡡����None
** ����������Can recevie task
*******************************************************************************************/
void CanRx_Task(void *p_arg)
{
	INT16S err;
	
	gUwbStart = OSSemCreate(0);

	while(1)
	{
		err = DevBusRead(0, (void *)&gCanRxcData, sizeof(sCAN_FRAME));
 		if(err == CANBUS_RX_TIMEOVER)
 		{
 			gUwbState = UWB_INIT;					//change uwb module of state 
 		}
 		else 
		{
		 	CanRxProc(gCanRxcData);
		}
	}
}
/******************************************************************************
** ��������: 
** ��������: ��ʼ���豸��������ݽṹ
** ����������CAN APP ��ʼ��
*******************************************************************************/
void CanAppInit(void)
{
	INT8U err;
	
	CanBusDataInit(CAN_DEV_ID);
	
#if CAN_TASK_MODE == CAN_MONITOR_MODE
	gUwbRunPara.extent = 100;
#endif /*CAN_TASK_MODE == CAN_MONITOR_MODE*/
	err = OSTaskCreateExt((void (*)(void *))CanRx_Task,
					(void          * )0,
					(OS_STK        * )&CanRx_stk[CANRX_TASK_SIZE - 1],
					(uint8_t         )CANRX_TASK_PRIO,
					(uint16_t        )CANRX_TASK_PRIO,
					(OS_STK        * )&CanRx_stk[0],
					(INT32U          )CANRX_TASK_SIZE,
					(void          * )0,
					(uint16_t        )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
	
	if (err == OS_ERR_NONE)
	{
		OSTaskNameSet(CANRX_TASK_PRIO, (uint8_t *)"Can Rx Task", &err);
	}
	if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)		//ʹ����Ա��λ����
	{
		err = OSTaskCreateExt((void (*)(void *))CanTx_Task,
						(void          * )0,
						(OS_STK        * )&CanTx_stk[CANTX_TASK_SIZE - 1],
						(uint8_t         )CANTX_TASK_PRIO,
						(uint16_t        )CANTX_TASK_PRIO,
						(OS_STK        * )&CanTx_stk[0],
						(INT32U          )CANTX_TASK_SIZE,
						(void          * )0,
						(uint16_t        )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
	}
	if (err == OS_ERR_NONE)
	{
		OSTaskNameSet(CANTX_TASK_PRIO, (uint8_t *)"CAN TX TASK", &err);
	}

}

