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
*变量定义
*********************************************************************************/

OS_STK CanRx_stk[CANRX_TASK_SIZE];
OS_STK CanTx_stk[CANTX_TASK_SIZE];

uint32_t gUwbState = 0;				//UWB信息初始配置状态,=0表示未进行配置，=1表示配置完成

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

uint16_t gWlAddr = 0;				//WLM配置地址

/*******************************************************************************************
**函数名称：CRC_8(u8 *PData, u8 Len)
**输　入：None
**输　出：None
**功能描述：Can发送任务
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
* 功能描述：发送角度数据帧
 * 输入参数：无
 * 创建时间：
 * 创建作者：
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
		case ANGLE_REPORT_VALUE:			//角度上报帧
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
			
		case ANGLE_SET_DEV_TYPE:			//设置设备类型/应答
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
			
		case ANGLE_CHECK_VALUE:					//角度查询帧
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
		
		case ANGLE_SET_MODIFY_PARAM_NUMB:		//设置出厂修正值总数
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
			
		case ANGLE_GET_DEV_TYPE_ACK:		//获取设备类型应答
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
		
		case ANGLE_SET_MODIFY_VALUE:			//设置出厂修正值应答
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
			
		case ANGLE_GET_MODIFY_VALUE_ACK:			//回读出厂修正值应答
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

		case ANGLE_MODIFY_PARAM_SAVE:			//出厂修正值参数保存
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
			LogicRunInfApi(LOGIC_GET_WORK_MODE, &u16VauleTemp);			//工作模式
			ret.Data[1] = (u8)(u16VauleTemp & 0xFF);
			LogicRunInfApi(LOGIC_GET_REPORT_INTERVAL, &u16VauleTemp);		//角度上报间隔
			ret.Data[2] = (u8)(u16VauleTemp & 0xFF);
			ret.Data[3] = (u8)((u16VauleTemp & 0xFF00) >> 8);
			ret.Data[4] = 0x00;
			ret.Data[5] = 0x00;
			ret.Data[6] = 0x00;
			ret.Data[7] = 0x00;
		break;
		
		case ANGLE_READ_VERSION_ACK:		//回读版本号应答
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
			ret.Data[5] = VERSION_1;   //版本号
			ret.Data[6] = VERSION_2;   //版本号
			ret.Data[7] = VERSION_3;   //版本号
		break;
		
		default:
			break;
	}
	return ret;
		
}
/************************************************************************************************
* 功能描述：发送UWB数据帧
 * 输入参数：无
 * 创建时间：
 * 创建作者：
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
		case UWB_CONFIG_REQ:			//参数请求
		case UWB_HEART:					//人员定位模块心跳
			tmp.ID.TID = UWB_ID;
			memcpy(ret.Data, "ALM", 3);
			ret.Data[3] = UWB_MODEL;          //指定为人员定位模块
			ret.Data[4] = (func == UWB_CONFIG_REQ) ? 1 : 0;
			ret.Data[5] = VERSION_1;   //版本号
			ret.Data[6] = VERSION_2;   //版本号
			ret.Data[7] = VERSION_3;   //版本号
			ret.DLC = 8;
			ret.Stdid = tmp.u32Id;
			break;

		case UWB_INFO_REPORT:			//1：人员定位信息上报
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
* 功能描述：发送标签信息
 * 输入参数：无
 * 创建时间：
 * 创建作者：
*************************************************************************************************/
int32_t WriteReportInfo(uint8_t tagNum, uint16_t tagId, uint16_t dist, uint16_t state, uint8_t right)
{
	sTag_Info 	tmp;
	sCAN_FRAME  cantmp;
	
	memset(&tmp, 0x00, sizeof(tmp));
	tmp.peoplenum = tagNum;
	tmp.tagInfo.sTag.tagState = state;
	tmp.tagInfo.sTag.author = right;    //添加权限
	tmp.tagId = tagId;
	tmp.tagDist = dist;
	
	cantmp = SendUwbData(UWB_INFO_REPORT, 0, (uint8_t *)&tmp, 6);
	return CanBufferWrite(&gUwbCanTxBuf, cantmp);

}

/*******************************************************************************************
**函数名称：void CanTx_Task(void *p_arg)
**输　入：None
**输　出：None
**功能描述：Can发送任务
*******************************************************************************************/
#if CAN_TASK_MODE == CAN_POSITION_MODE
void CanTx_Task(void *p_arg)
{
    INT8U err;
    sCAN_FRAME sendData;
    CanBufferDataInit(&gUwbCanTxBuf);
    
    while (1)
    {
        if (gUwbState == UWB_INIT)			//人员定位模块初始未配置状态
        {
            STM32_CAN_Write(0, SendUwbData(UWB_CONFIG_REQ, 0, NULL, 0), CAN_FF_EXTENDED);
            OSTimeDly(REQ_CONFIG_DELAY);			//配置不成功，每间隔500ms申请一次参数
        }
        else								//人员定位模块初始配置完成
        {
            OSSemPend(gUwbCanTxBuf.dataSem, UWB_HEART_DELAY, &err);
            if (err == OS_ERR_NONE)
            {
                if (CanBufferRead(&gUwbCanTxBuf, &sendData) == 0)
                {
                    while (STM32_CAN_Write(0, sendData, CAN_FF_EXTENDED) == 1)
                    {
                        OSTimeDly(5); // 写发送数据
                    }
                }
                else//can发送buf满
                {
                    gUwbState= UWB_INIT;
                }
            }
            else if (err == OS_ERR_TIMEOUT)
            {
                STM32_CAN_Write(0, SendUwbData(UWB_HEART, 0, NULL, 0), CAN_FF_EXTENDED); // 写人员定位心跳帧
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
//					while(STM32_CAN_Write(0, sendData, CAN_FF_EXTENDED)) // 写发送数据
//					{
//						OSTimeDly(1);
//					}
//				}
//				else{//can发送buf满
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
	return gUwbRunPara.standNum;			//支架控制器编号
}

__inline uint16_t GetUwbReportTime(void)
{
	uint16_t tmp;
	tmp = gUwbRunPara.interval * 100;
	return tmp;
}
/*******************************************************************************************
**函数名称：int32_t CheckTagState(uint16_t dist)
**输　入：标签距离
**输　出：1：离开检测范围 0：在检测范围内
**功能描述：
*******************************************************************************************/
int32_t CheckTagState(uint16_t dist)
{
	return (dist > gUwbRunPara.extent) ? 1 : 0;
}


uint16_t GetWLMId(void)
{
	return gWlAddr;			//支架控制器编号
}
can_trasnmit_message_struct WLTxCan;
/***********************************************************************************************
** 函 数 名：	CanRcvIrMatchProc
** 输　  入：	
** 输　  出：	
** 功能描述：	接收支架控制器的红外对码结果数据，转成遥控器和WL的红外通讯协议帧
** 注意事项：	
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
static u32 CanRcvIrMatchProc(sCanFrame *sRxCan)
{
	u32 u32RetVal = SUCCESS,i;
	static u16 LastWlAddr = 0, WlAddr;
	IR_INFO_u	data;
	sCanFrame sTxCan;
	
	//如果需要应答发送应答帧
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
	
	WlAddr = ((u16)sRxCan->u8Data[4] << 8) + sRxCan->u8Data[3];//对码架号
	data.sIrInfo.Type = 2;									//命令类型
	data.sIrInfo.ScNoLSB3 = (((u32)sRxCan->u8Data[4] << 8) + sRxCan->u8Data[3]) & 0x7;//对码架号低3位
	data.sIrInfo.Sign1 = 1;									//起始标识
	data.sIrInfo.ScNoMSB6 = ((((u32)sRxCan->u8Data[4] << 8) + sRxCan->u8Data[3]) >> 3) & 0x3f;//对码架号低6位
	data.sIrInfo.Dir = eDirectHS;							//发送方向
	data.sIrInfo.Sign2 = 0;									//中间字节标识
	data.sIrInfo.Result = sRxCan->u8Data[2] & 0x01;			//对码结果
	data.sIrInfo.ACK = eACK;								//应答标识
	data.sIrInfo.RemoteID = sRxCan->u8Data[5] & 0xf;		//遥控器ID
	data.sIrInfo.ScNoIncDir = sRxCan->u8Data[1] & 0x01;		//支架控制器架号增向
	data.sIrInfo.Sign3 = 0;									//中间字节标识

	// 如果红外对码结果成功，点亮绿色指示灯；
	if(data.sIrInfo.Result == 0x01)
	{
		SaveSRemoteCtrlInfo((u8)data.sIrInfo.RemoteID);		//保存遥控器ID
//		SetLedState(LED_GREEN,TRUE,0xff);
	}
	
	// 收到的支架作为无线红外收发模块的无线地址
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
** 函 数 名：	CanRcvWlSendProc
** 输　  入：	
** 输　  出：	
** 功能描述：	接收支架控制器的无线对码数据，转成遥控器和 wl 的无线通讯协议帧
** 注意事项：	
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
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
	//如果需要应答发送应答帧
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
	//发送给遥控器
// 	st_WlTrs.u8Addr = GetRemoteIdByWl();		// 获取遥控器ID
	st_WlTrs.u8Addr = 255 - To;					// 获取遥控器接收地址
	st_WlTrs.u8Ack = sRxCan->u32ID.ID.ACK;
	st_WlTrs.u8FT = sRxCan->u32ID.ID.FT;
	st_WlTrs.u32Len = sRxCan->u16DLC + 1 + 4;	// 1表示CAN数据帧的长度占用一个字节，4表示CAN ID占用四个字节
	
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
	
	// 如果无线对码结果成功，点亮红色指示灯；
	//if(sRxCan->u8Data[2] == 0x01 && (sRxCan->u32ID.ID.FT == FT_SC_TO_WL_RF_MATCH_RST))
	if((sRxCan->u8Data[2] & WL_MATCH_RESULT_MASK) == WL_MATCH_OK && (sRxCan->u32ID.ID.FT == FT_SC_TO_WL_RF_MATCH_RST))
	{
		SaveSRemoteCtrlInfo((u8)To);			//保存遥控器ID
//		SetLedState(LED_GREEN,FALSE,0x00);
//		SetLedState(LED_RED,TRUE,0xff);
	}
	
	// 如果解除支架控制器发送来的对码解除数据帧，熄灭指示灯
	if(sRxCan->u32ID.ID.FT == FT_SC_TO_WL_DISCONNECT)
	{
//		SetLedState(LED_GREEN,FALSE,0x00);
//		SetLedState(LED_RED,FALSE,0x00);
	}
	return u32RetVal;
}
/***********************************************************************************************
** 函 数 名：	CanRcvSetPar
** 输　  入：	
** 输　  出：	
** 功能描述：	接收支架控制器设置参数帧，进行相应处理
** 注意事项：	
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
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
	u32 u32SN = 0xffffffff;			//上电初始化序列号
	
	if(u32SN == (u32)sRxCan->u32ID.ID.SN)
		return FALSE;
	else
		u32SN = sRxCan->u32ID.ID.SN;
    

	/*
	 * 设置架号
	 */
    SetWlAddr((u8)sRxCan->u8Data[0]);
	RfidWriteReg(CC1101_ADDR, gWlAddr);
	/*
	 * 设置无线同步字
	 */
	SC_RFID_SYNC0 = (u8)sRxCan->u8Data[1];
	RfidWriteReg(CC1101_SYNC0, SC_RFID_SYNC0); // SYNC0
	SC_RFID_SYNC1 = (u8)sRxCan->u8Data[2];
	RfidWriteReg(CC1101_SYNC1, SC_RFID_SYNC1); // SYNC
    
    
	
//	u16Offset = (u16)sRxCan->u8Data[0] | (u16)(sRxCan->u8Data[1] << 8);
	
	/*
	 * 设置无线同步字
	 */
//	if(u16Offset == WL_SYNC)
//	{
//		SetSync((u32)sRxCan->u8Data[2]|(u32)(sRxCan->u8Data[4]<<8));
//		return TRUE;
//	}

//	k = (sRxCan->u16DLC - 2) >> 1;			// 实际参数个数

//	for(i = 0;i < k;i++)
//	{
//		FlushPTPar(u16Offset + i,(u16)sRxCan->u8Data[2 + (i<<1)], (u16)sRxCan->u8Data[3 + (i<<1)]);
//	}
	
//	gWlAddr = (u32)sRxCan->u8Data[2] | (u32)(sRxCan->u8Data[4] << 8);
	/*
	 * 设置架号，如果是服务器指定为0xFF
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
** 函 数 名：	CanRcvBusProc
** 输　  入：	
** 输　  出：	
** 功能描述：	处理支架控制器发来的CAN总线数据
** 注意事项：	
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
static st_WL_TRANS_DATA st_WlTrsTmp;						//保存空中传播总线数据
static u32 u32WlBusDataLen = 0;							    //记录已填充无线总线数据个数
static u32 u32CanToWlFrmNum = 0;							//要转化为无线数据的CAN数据帧个数
static u32 u32CurFrm = 0;									//记录当前帧，与下一帧比较
static u32 CanRcvBusProc(sCanFrame *sRxCan)
{
//	u32 u32RetVal = TRUE;
	u32 u32SN = 0xffffffff;			//上电初始化序列号
	sCanFrame sTxCan;
	u32 i,j;
	//重复帧过滤
	if(u32SN == sRxCan->u32ID.ID.SN)
		return FALSE;
	else
		u32SN = sRxCan->u32ID.ID.SN;
	
	//如果需要应答发送应答帧
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
	 * 处理总帧,总帧数=u32ID.ID.SUM
	 */
	if((sRxCan->u32ID.ID.SUB == 0) && (sRxCan->u32ID.ID.SUM > 0))
	{
		/*
		 * 记录收到CAN帧的子帧个数
		 */
		u32CanToWlFrmNum = sRxCan->u32ID.ID.SUM;//组合帧的总帧数
		u32WlBusDataLen = 0;
		u32CurFrm = 0;
		/*
		 * 装配无线总线数据在空中传播的帧ID
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
		 * 装配空中无线总线数据的帧ID数据
		 */
		for (i=0;i<4;i++)
		{
			st_WlTrsTmp.u8WlData[i] = (((u8)(st_WlTrsTmp.u32ID.u32Id >> (i << 3))) & 0xff);//29位扩展帧ID，占用4个字节
		}
		
		// 取出要发送的地址
		st_WlTrsTmp.u8Addr = sRxCan->u8Data[0];		// 取出支架控制器要发送的无线地址作为无线红外模块的发送地址
		
		st_WlTrsTmp.u8WlData[i] = u32WlBusDataLen; //数据长度
		
		for(j=0;j<sRxCan->u16DLC;j++)
		{
			st_WlTrsTmp.u8WlData[i + 1 + j] = sRxCan->u8Data[j];
		}
		u32WlBusDataLen += sRxCan->u16DLC;
		
		return FALSE;
	}
	/*
	 * 处理子帧,如果子帧之间不连续，则丢弃
	 */
	if(sRxCan->u32ID.ID.SUB == 1)
	{
		if(u32CurFrm < u32CanToWlFrmNum)
		{
			if(u32CurFrm == sRxCan->u32ID.ID.SUM)//应接收的子帧序号和接收到的子帧序号相同
			{
				for(i=0;i<sRxCan->u16DLC;i++)//复制数据
				{
					st_WlTrsTmp.u8WlData[u32WlBusDataLen + i + 5] = sRxCan->u8Data[i];
				}
				u32WlBusDataLen += sRxCan->u16DLC;
				u32CurFrm++;//下一帧应接收的子帧序号
			}
			if(u32CurFrm == u32CanToWlFrmNum)//组合帧接收完成
			{
				/*
				 * 将无线总线数据转发给服务器510，实际器件地址0xFF
				 */
				//st_WlTrsTmp.u8Addr = 0xFF;		// 服务器地址
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
//校准X、Y角度值
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
	
	if (tmp.ID.RD == RD_GLOBAL_FRAME)//数据帧
	{
		//使能人员定位功能
		if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
		{
			if (tmp.ID.FT == SET_UWB_PARA && tmp.ID.RID == UWB_ID)		//设置人员定位模块工作参数
			{
				memcpy(&gUwbRunPara, CanRX_Proc.Data, sizeof(gUwbRunPara));
				gUwbState = UWB_NORMAL;
				OSSemPost(gUwbStart);
			}
		}
		
	/**发送给角度传感器的数据**/
	#if 1
	//使能角度采集功能
	if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
	{
		if (tmp.ID.RID == ANGLE_ID)				//数据帧
		{
			switch(tmp.ID.FT)
			{
				case ANGLE_CHECK_VALUE:				//角度查询帧
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break;
					
					STM32_CAN_Write(0, SendAngleData(ANGLE_CHECK_VALUE, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED); // 进行应答
					u16CanLeftSendTimer = 0x00;
				}
				break;

				case ANGLE_SET_DEV_TYPE:			//设置设备类型
				{
					if(CanRX_Proc.DLC != CAN_LENGTH_8)
						break;
			
					u32Sum = SYSTEM_RUN_STATUS_ADJUST;
					LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);

					if(LogicParamApi(LOGIC_SET_DEVTYPE_INF, &(CanRX_Proc.Data[0x00])))	
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_SET_DEV_TYPE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答	

					}
				}
				break;

				case ANGLE_GET_DEV_TYPE:			//获取设备类型
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break ;			

					if(LogicParamApi(LOGIC_GET_DEVTYPE_INF, &(CanRX_Proc.Data[0x00])))	
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_GET_DEV_TYPE_ACK, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答
					}
					u16CanLeftSendTimer = 0x00;
				}
				break;
					
				case ANGLE_SET_MODIFY_PARAM_NUMB:			//设置出厂修正值总数
				{
					if(CanRX_Proc.DLC != CAN_LENGTH_6)
						break ;
					
					u32Sum = SYSTEM_RUN_STATUS_ADJUST;
					LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);

					if(LogicParamApi(LOGIC_SET_MODIFY_PARAM_NUMB, &(CanRX_Proc.Data[0x02])))	
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_SET_MODIFY_PARAM_NUMB, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答	
					}
				}
				break;
					
				case ANGLE_GET_MODIFY_PARAM_NUMB:			//回读出厂修正值总数
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break ;	
						
					if(LogicParamApi(LOGIC_GET_MODIFY_PARAM_NUMB, &(CanRX_Proc.Data[0x02])))	
					{				
						STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_PARAM_NUMB_ACK, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答	
					}
					u16CanLeftSendTimer = 0x00;
				}
				break;

				case ANGLE_SET_MODIFY_VALUE:			//设置出厂修正值
				{
					if(CanRX_Proc.DLC != CAN_LENGTH_8)
						break ;
					
					u32Sum = SYSTEM_RUN_STATUS_ADJUST;
					LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);
					
					AngleSensorCalibrateXYFunction((LOGIC_SET_ANGLEPARAM_TYPE *)&(CanRX_Proc.Data[0x00]));
					if(LogicParamApi(LOGIC_SET_MODIFY_PARAM, &(CanRX_Proc.Data[0x00])))	
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_SET_MODIFY_VALUE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答	
					}
				}
				break;

				case ANGLE_GET_MODIFY_VALUE:			//回读出厂修正值
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
							STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_VALUE_ACK, tmp.ID.TID, (uint8_t *)&SendFrame.u8DT, 0), CAN_FF_EXTENDED);		//进行应答
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
							STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_VALUE_ACK, tmp.ID.TID, (uint8_t *)&SendFrame.u8DT, 0), CAN_FF_EXTENDED);		//进行应答
						}
						OSTimeDly(10);
					}	
					u16CanLeftSendTimer = 0x00;	
				}
				break;

				case ANGLE_MODIFY_PARAM_SAVE:			//出厂修正值参数保存
				{
					if(CanRX_Proc.DLC != CAN_LENGTH_2)
						break ;
					
					u32Sum = SYSTEM_RUN_STATUS_ADJUST;
					LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);
					
					if(CanRX_Proc.Data[0x01] != 0x01)
						break;
					if(LogicParamApi(LOGIC_SAVE_PARAM_MSG, NULL))
					{
						STM32_CAN_Write(0, SendAngleData(ANGLE_MODIFY_PARAM_SAVE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答	
						OSTimeDly(10);
						//执行跳转
						IapJumpToBoot(IN_FLASH_BOOTLOADER_ADDR);
					}
				}
				break;

				case ANGLE_SET_WORK_PARAM:					//设置工作参数
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
						
					u16CanLeftSystemRunStatus = SYSTEM_RUN_STATUS_NORMAL;    //矫贺岩2020/03/09
					
		//			u8LeftActiveReportAngleChange = CanRX_Proc.Data[0x05];           //矫贺岩2020/03/09
					/*************************单can
					CanRightSetTemporaryStatus(u16CanLeftSystemRunStatus, u32LeftReportRxDevId, u8LeftActiveReportAngleChange);
					*****************************/
					
					STM32_CAN_Write(0, SendAngleData(ANGLE_SET_WORK_PARAM, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//进行应答	
				}
				break;

				case ANGLE_GET_WORK_PARAM:			//回读工作参数
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break ;

					STM32_CAN_Write(0, SendAngleData(ANGLE_GET_WORK_PARAM_ACK, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//进行应答
					u16CanLeftSendTimer = 0x00;
				}
				break;

				case ANGLE_READ_VERSION:			//读取版本号
				{
					if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
						break ;

					STM32_CAN_Write(0, SendAngleData(ANGLE_READ_VERSION_ACK, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//进行应答
					u16CanLeftSendTimer = 0x00;	
				}
				break;

				default:
					break;
			}
		}
	}
	#endif
	
		/**发送给声光报警器的数据，需进行处理**/
		if ((CanRX_Proc.Stdid == FRM_ID_BEFORE_ALARM) && (CanRX_Proc.DLC == 0x05))		// 声光报警器预警帧ID
		{
			BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;					//收到预警预警命令，清超时定时器
			Led_PowerOn();
			SetCurrentLightType(LED_CLASSIC_MODE);				//声光报警驱动类型
			if ((CanRX_Proc.Data[0] == 0x01) && (CanRX_Proc.Data[1] == 0x01))			//第一个字节为0x01
			{
				BeepCaseFlag = CanRX_Proc.Data[0];							//光
				LightFlag = CanRX_Proc.Data[1];								//声
			}
			else if ((CanRX_Proc.Data[0] == AGING_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//第一个字节用于设备老化LED使用，点亮4个LED，不老化蜂鸣器
			{		//设备类型
				LightFlag = AGING_TEST_BYTE;				//灯常亮
				BeepCaseFlag = 0;							//不发声
			}
			else if ((CanRX_Proc.Data[0] == POWER_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//第一个字节用于设备检验，测试整机功耗，点亮4个LED，驱动蜂鸣器
			{
				LightFlag = POWER_TEST_BYTE;					//灯常亮
				BeepCaseFlag = 0xffff;							//长发声
			}
			else if ((CanRX_Proc.Data[0] == LED_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//第一个字节用于设备检验，测试LED是否正常，顺序点亮三色LED，驱动蜂鸣器
			{
				LightFlag = LED_TEST_BYTE;						//灯常亮
				BeepCaseFlag = 0;								//不发声
			}
			else
			{
				if((CanRX_Proc.Data[1] == 0x10)
				|| (CanRX_Proc.Data[1] == 0x11)
				|| (CanRX_Proc.Data[1] == 0x12)
				|| (CanRX_Proc.Data[1] == 0x21)
				|| (CanRX_Proc.Data[1] == 0x22)
				|| (CanRX_Proc.Data[1] == 0x23))    //蓝灯常亮
				{
					LightFlag = CanRX_Proc.Data[1];           						//光
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
				|| (CanRX_Proc.Data[0] == 0x01))    //蓝灯常亮
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
		else if ((CanRX_Proc.Stdid == FRM_ID_ACTION_ALARM) && (CanRX_Proc.DLC == 0x05))		// 声光报警器动作报警帧ID
		{
			BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;			//收到动作预警命令，清超时定时器
			Led_PowerOn();
			SetCurrentLightType(LED_CLASSIC_MODE);				//声光报警驱动类型
			if ((CanRX_Proc.Data[0] == 0x01) 
			&& (CanRX_Proc.Data[1] == 0x02))			//第一个字节为0x01
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
				|| (CanRX_Proc.Data[1] == 0x23))    //蓝灯常亮
				{
					LightFlag = CanRX_Proc.Data[1];           						//光
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
				|| (CanRX_Proc.Data[0] == 0x01))    //蓝灯常亮
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
		else if ((CanRX_Proc.Stdid == FRM_ID_ACTION_ALARM) && (CanRX_Proc.DLC == 0x08))		// 声光报警器动作报警帧ID
		{
			BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;			//收到动作预警命令，重置超时定时器
			Led_PowerOn();
			if (CanRX_Proc.Data[0] == 0x01)			//第一个字节为0x01
			{
				SetCurrentLightType(LED_EXTEND_MODE);				//声光报警驱动类型
				LightFlag = CanRX_Proc.Data[0];		//LED驱动工作模式
				RGrayScale = CanRX_Proc.Data[1];
				GGrayScale = CanRX_Proc.Data[2];
				BGrayScale = CanRX_Proc.Data[3];
				u16LightOnTimer = (u16)(CanRX_Proc.Data[4] * 10);
				u16LightOffTimer = (u16)(CanRX_Proc.Data[5] * 10);
				if(CanRX_Proc.Data[6] == 0)
				{
					BeepCaseFlag = 1;		//标准发声逻辑
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
					BeepCaseFlag = 0x11;		//可配置发声逻辑
					u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
					u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
				}
			}
			else if(CanRX_Proc.Data[0] == 0x02)		//流水灯效果
			{
				SetCurrentLightType(LED_EXTEND_MODE);				//声光报警驱动类型
				LightFlag = CanRX_Proc.Data[0];		//LED驱动工作模式
				RGrayScale = CanRX_Proc.Data[1];
				GGrayScale = CanRX_Proc.Data[2];
				BGrayScale = CanRX_Proc.Data[3];
				LED_NUM = ((CanRX_Proc.Data[4] & 0xf0) >> 4);
				WaterFlow_Type = CanRX_Proc.Data[4] & 0x0f;
				u16LightOnTimer = (u16)(CanRX_Proc.Data[5] * 10);
				if(CanRX_Proc.Data[6] == 0)
				{
					BeepCaseFlag = 1;		//标准发声逻辑
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
					BeepCaseFlag = 0x11;		//可配置发声逻辑
					u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
					u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
				}
			}
			else if(CanRX_Proc.Data[0] == 0x03)		//呼吸灯效果
			{
				SetCurrentLightType(LED_EXTEND_MODE);				//声光报警驱动类型
				LightFlag = CanRX_Proc.Data[0];		//LED驱动工作模式
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
					BeepCaseFlag = 1;		//标准发声逻辑
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
					BeepCaseFlag = 0x11;		//可配置发声逻辑
					u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
					u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
				}
			}
			else if(CanRX_Proc.Data[0] == 0x04)			//流星灯效果
			{
				SetCurrentLightType(LED_EXTEND_MODE);				//声光报警驱动类型
				LightFlag = CanRX_Proc.Data[0];		//LED驱动工作模式
				RGrayScale = CanRX_Proc.Data[1];
				GGrayScale = CanRX_Proc.Data[2];
				BGrayScale = CanRX_Proc.Data[3];
				LED_NUM = CanRX_Proc.Data[4];
				u16LightOnTimer = (u16)(CanRX_Proc.Data[5] * 10);
				if(CanRX_Proc.Data[6] == 0)
				{
					BeepCaseFlag = 1;		//标准发声逻辑
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
					BeepCaseFlag = 0x11;		//可配置发声逻辑
					u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
					u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
				}
			}
			else if(CanRX_Proc.Data[0] == 0x05)			//多彩灯效果
			{
				SetCurrentLightType(LED_EXTEND_MODE);				//声光报警驱动类型
				LightFlag = CanRX_Proc.Data[0];		//LED驱动工作模式
				RGrayScale = CanRX_Proc.Data[1];
				GGrayScale = CanRX_Proc.Data[1];
				BGrayScale = CanRX_Proc.Data[1];		//只使用同一种灰度数值用于混色时的值是1:1，否则比例不好颜色不正。
				LED_NUM = (CanRX_Proc.Data[2] & 0x3f);
				u16LightOnTimer = (u16)(CanRX_Proc.Data[3] * 10);
				if(CanRX_Proc.Data[6] == 0)
				{
					BeepCaseFlag = 1;		//标准发声逻辑
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
					BeepCaseFlag = 0x11;		//可配置发声逻辑
					u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
					u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
				}
			}
			else
			{
				;
			}
		}
		
	/**发送给红外接收的数据**/
	#if 1
	if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
	{
		if (tmp.ID.RID == WL_RID)				//数据帧
		{
			switch (tmp.ID.FT)
			{
				case FT_SC_TO_WL_DBUS:							//2				// SC 发给 WL 的总线数据，需要应答
					CanRcvBusProc(&IRCanRX_Proc);
				break;
				case FT_WL_TO_SC_DBUS:							//3				// WL 发给 SC 的总线数据，需要应答
					
				break;
				case FT_SC_TO_WL_IR_MATCH_RST:					//5				// SC 发给 WL 的红外对码结果，需要应答
					CanRcvIrMatchProc(&IRCanRX_Proc);
				break;
				
				case FT_SC_TO_WL_RF_MATCH_RST:					//7				// SC 发给 WL 的无线对码结果，需要应答
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
				break;
				case FT_SC_TO_WL_MATCH_SC_GROUP_INFO:			//8				// SC发送给WL，对码架发送成组信息	
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
				break;
				case FT_SC_TO_WL_UC_SC_GROUP_INFO:				//9				// SC发送给WL，被控架发送成组信息
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
				break;
				case FT_SC_TO_WL_CTL_DATA_RECEPT:				//11			// SC 发送给 遥控器的控制数据是否接收，需要应答			
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
				break;
				case FT_SC_TO_WL_DISCONNECT:					//14			// SC 发送给 WL 解除对码
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
				break;
				case FT_SC_TO_WL_RESET_PAR:						//16			// SC 发送给 WL 重设参数
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
				break;
				case FT_SC_TO_WL_LIFT_RECEPT:					//17			// SC 发送给 WL 按键抬起是否接受
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
				break;
				
				case FT_WL_TO_SC_AUTO_PRESS_CLOSE:					//20			// SC 发送给 WL 按键抬起是否接受
					CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[0]);
				break;				
				case FT_SC_TO_WL_RESET_PAR_WL:					//22			// 设置无线红外接收模块参数
					CanRcvSetPar(&IRCanRX_Proc);						//PT_SER_SC_NO;
				break;
//				case FT_SC_TO_WL_MATCH_END:						//31			// 对码直接结束
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
	else if (tmp.ID.RD == RD_PRG_FRAME)			//程序更新数据帧
	{
		u32CanLeftPrgRecvTimer = CANLEFT_PRG_SEND_TIMEOUT;

		stFrame.u32ID.u32Id = tmp.u32Id;
		stFrame.u16DLC = CanRX_Proc.DLC;
		memcpy(stFrame.u8DT, CanRX_Proc.Data, stFrame.u16DLC);

		if((stFrame.u32ID.ID.PACKET_NUMB) == 0x01)			//收到程序更新第一包
		{	
			if((stFrame.u32ID.ID.TD_FTYPE) == CXB_FRAME_DOWNLOAD_PRG_VERSION)
			{
				u32Sum = SYSTEM_RUN_STATUS_UPPRG;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);				
				//使能角度采集功能
				if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
				{
					/**挂起不必要的任务**/
					OSTaskQuery(ANGLE_SENSOR_TASK_PRIO, &pdata);			//查询处理任务是否挂起
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(ANGLE_SENSOR_TASK_PRIO);
					}
				}
				//使能人员定位功能
				if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
				{
					/**挂起不必要的任务**/
					OSTaskQuery(UWBAPP_TASK_PRIO, &pdata);			//查询处理任务是否挂起	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{
						OSTaskSuspend(UWBAPP_TASK_PRIO);
					}
					/**挂起不必要的任务**/
					OSTaskQuery(CANTX_TASK_PRIO, &pdata);			//查询处理任务是否挂起	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(CANTX_TASK_PRIO);
					}
				}
				//使能433无线模块功能
				if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
				{
					/**挂起不必要的任务**/
					OSTaskQuery(WL_MANAGE_TASK_PRIO, &pdata);			//查询处理任务是否挂起	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(WL_MANAGE_TASK_PRIO);
					}
				}
			}
		}
		//收到传输的程序帧时的处理
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
	
	
	if (tmp.ID.RD == RD_GLOBAL_FRAME)				//数据帧
	{
		//使能人员定位功能
		if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
		{
			if (tmp.ID.RID == UWB_ID)
			{
				switch (tmp.ID.FT)
				{
					case REQ_UWB_INFO:			//2：查询人员定位模块的标签距离
						break;

					case SET_UWB_PARA:
						if(tmp.ID.RID == UWB_ID)
						{
							memcpy(&gUwbRunPara, CanRX_Proc.Data, sizeof(gUwbRunPara));
							CanRxParaProcess();
						}
						break;

					case UWB_RESET:		//6：复位人员定位模块或声光报警器
						if(tmp.ID.RID == UWB_ID)
						{
							NVIC_SystemReset();
						}
						
					default:
						break;
				}
			}
		}
	
/**发送给角度传感器的数据**/
//#if 0
	/**发送给角度传感器的数据，需进行处理**/
	if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
	{
		if (tmp.ID.RID == ANGLE_ID)
		{
		switch(tmp.ID.FT)
		{
			case ANGLE_CHECK_VALUE:				//角度查询帧
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break;
				
				STM32_CAN_Write(0, SendAngleData(ANGLE_CHECK_VALUE, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED); // 进行应答
				u16CanLeftSendTimer = 0x00;
			}
			break;

			case ANGLE_SET_DEV_TYPE:			//设置设备类型
			{
				if(CanRX_Proc.DLC != CAN_LENGTH_8)
					break;
		
				u32Sum = SYSTEM_RUN_STATUS_ADJUST;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);

				if(LogicParamApi(LOGIC_SET_DEVTYPE_INF, &(CanRX_Proc.Data[0x00])))	
				{
					STM32_CAN_Write(0, SendAngleData(ANGLE_SET_DEV_TYPE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答	

				}
			}
			break;

			case ANGLE_GET_DEV_TYPE:			//获取设备类型
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break ;			

				if(LogicParamApi(LOGIC_GET_DEVTYPE_INF, &(CanRX_Proc.Data[0x00])))	
				{
					STM32_CAN_Write(0, SendAngleData(ANGLE_GET_DEV_TYPE_ACK, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答
				}
				u16CanLeftSendTimer = 0x00;
			}
			break;
				
			case ANGLE_SET_MODIFY_PARAM_NUMB:			//设置出厂修正值总数
			{
				if(CanRX_Proc.DLC != CAN_LENGTH_6)
					break ;
				
				u32Sum = SYSTEM_RUN_STATUS_ADJUST;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);

				if(LogicParamApi(LOGIC_SET_MODIFY_PARAM_NUMB, &(CanRX_Proc.Data[0x02])))	
				{
					STM32_CAN_Write(0, SendAngleData(ANGLE_SET_MODIFY_PARAM_NUMB, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答	
				}
			}
			break;
				
			case ANGLE_GET_MODIFY_PARAM_NUMB:			//回读出厂修正值总数
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break ;	
					
				if(LogicParamApi(LOGIC_GET_MODIFY_PARAM_NUMB, &(CanRX_Proc.Data[0x02])))	
				{				
					STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_PARAM_NUMB_ACK, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答	
				}
				u16CanLeftSendTimer = 0x00;
			}
			break;

			case ANGLE_SET_MODIFY_VALUE:			//设置出厂修正值
			{
				if(CanRX_Proc.DLC != CAN_LENGTH_8)
					break ;
				
				u32Sum = SYSTEM_RUN_STATUS_ADJUST;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);
				
				AngleSensorCalibrateXYFunction((LOGIC_SET_ANGLEPARAM_TYPE *)&(CanRX_Proc.Data[0x00]));
				if(LogicParamApi(LOGIC_SET_MODIFY_PARAM, &(CanRX_Proc.Data[0x00])))	
				{
					STM32_CAN_Write(0, SendAngleData(ANGLE_SET_MODIFY_VALUE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答	
				}
			}
			break;

			case ANGLE_GET_MODIFY_VALUE:			//回读出厂修正值
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
						STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_VALUE_ACK, tmp.ID.TID, (uint8_t *)&SendFrame.u8DT, 0), CAN_FF_EXTENDED);		//进行应答
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
						STM32_CAN_Write(0, SendAngleData(ANGLE_GET_MODIFY_VALUE_ACK, tmp.ID.TID, (uint8_t *)&SendFrame.u8DT, 0), CAN_FF_EXTENDED);		//进行应答
					}
					OSTimeDly(10);
				}
				u16CanLeftSendTimer = 0x00;	
			}
			break;

			case ANGLE_MODIFY_PARAM_SAVE:			//出厂修正值参数保存
			{
				if(CanRX_Proc.DLC != CAN_LENGTH_2)
					break ;
				
				u32Sum = SYSTEM_RUN_STATUS_ADJUST;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);
				
				if(CanRX_Proc.Data[0x01] != 0x01)
					break;
				if(LogicParamApi(LOGIC_SAVE_PARAM_MSG, NULL))
				{
					STM32_CAN_Write(0, SendAngleData(ANGLE_MODIFY_PARAM_SAVE, tmp.ID.TID, (uint8_t *)&CanRX_Proc.Data, 0), CAN_FF_EXTENDED);		//进行应答	
					OSTimeDly(10);
					//执行跳转
					IapJumpToBoot(IN_FLASH_BOOTLOADER_ADDR);			//复位
				}
			}
			break;

			case ANGLE_SET_WORK_PARAM:					//设置工作参数
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
					
				u16CanLeftSystemRunStatus = SYSTEM_RUN_STATUS_NORMAL;    //矫贺岩2020/03/09
				
	//			u8LeftActiveReportAngleChange = CanRX_Proc.Data[0x05];           //矫贺岩2020/03/09
				/*************************单can
				CanRightSetTemporaryStatus(u16CanLeftSystemRunStatus, u32LeftReportRxDevId, u8LeftActiveReportAngleChange);
				*****************************/
				
				STM32_CAN_Write(0, SendAngleData(ANGLE_SET_WORK_PARAM, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//进行应答	
			}
			break;

			case ANGLE_GET_WORK_PARAM:			//回读工作参数
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break;

				STM32_CAN_Write(0, SendAngleData(ANGLE_GET_WORK_PARAM_ACK, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//进行应答
				u16CanLeftSendTimer = 0x00;
			}
			break;

			case ANGLE_READ_VERSION:			//读取版本号
			{
				if((CanRX_Proc.DLC != CAN_LENGTH_0) && (CanRX_Proc.DLC != CAN_LENGTH_8))
					break;

				STM32_CAN_Write(0, SendAngleData(ANGLE_READ_VERSION_ACK, tmp.ID.TID, NULL, 0), CAN_FF_EXTENDED);		//进行应答
				u16CanLeftSendTimer = 0x00;	
			}
			break;

			default:
				break;
		}
		}
	}


//#endif
	
	/**发送给声光报警器的数据，需进行处理**/
	if ((CanRX_Proc.Stdid == FRM_ID_BEFORE_ALARM) && (CanRX_Proc.DLC == 0x05))		// 声光报警器预警帧ID
	{
		BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;					//收到预警预警命令，清超时定时器
		Led_PowerOn();
		SetCurrentLightType(LED_CLASSIC_MODE);				//声光报警驱动类型
		if ((CanRX_Proc.Data[0] == 0x01) && (CanRX_Proc.Data[1] == 0x01))			//第一个字节为0x01
		{
			BeepCaseFlag = CanRX_Proc.Data[0];							//光
			LightFlag = CanRX_Proc.Data[1];								//声
		}
		else if ((CanRX_Proc.Data[0] == AGING_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//第一个字节用于设备老化LED使用，点亮4个LED，不老化蜂鸣器
		{		//设备类型
			LightFlag = AGING_TEST_BYTE;				//灯常亮
			BeepCaseFlag = 0;							//不发声
		}
		else if ((CanRX_Proc.Data[0] == POWER_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//第一个字节用于设备检验，测试整机功耗，点亮4个LED，驱动蜂鸣器
		{
			LightFlag = POWER_TEST_BYTE;					//灯常亮
			BeepCaseFlag = 0xffff;							//长发声
		}
		else if ((CanRX_Proc.Data[0] == LED_TEST_BYTE) && (CanRX_Proc.Data[1] == 0x01))			//第一个字节用于设备检验，测试LED是否正常，顺序点亮三色LED，驱动蜂鸣器
		{
			LightFlag = LED_TEST_BYTE;						//灯常亮
			BeepCaseFlag = 0;							//不发声
		}
		else
		{
			if ((CanRX_Proc.Data[1] == 0x10)
			|| (CanRX_Proc.Data[1] == 0x11)
			|| (CanRX_Proc.Data[1] == 0x12)
			|| (CanRX_Proc.Data[1] == 0x21)
			|| (CanRX_Proc.Data[1] == 0x22)
			|| (CanRX_Proc.Data[1] == 0x23))    //蓝灯常亮
			{
				LightFlag = CanRX_Proc.Data[1];           						//光
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
			|| (CanRX_Proc.Data[0] == 0x01))    //蓝灯常亮
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
	else if ((CanRX_Proc.Stdid == FRM_ID_ACTION_ALARM) && (CanRX_Proc.DLC == 0x05))		// 声光报警器动作报警帧ID
	{
		BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;			//收到动作预警命令，清超时定时器
		Led_PowerOn();
		SetCurrentLightType(LED_CLASSIC_MODE);				//声光报警驱动类型
		if ((CanRX_Proc.Data[0] == 0x01) 
		&& (CanRX_Proc.Data[1] == 0x02))			//第一个字节为0x01
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
			|| (CanRX_Proc.Data[1] == 0x23))    //蓝灯常亮
			{
				LightFlag = CanRX_Proc.Data[1];           						//光
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
			|| (CanRX_Proc.Data[0] == 0x01))    //蓝灯常亮
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
	else if ((CanRX_Proc.Stdid == FRM_ID_ACTION_ALARM) && (CanRX_Proc.DLC == 0x08))		// 声光报警器动作报警帧ID
	{
		BeepLightRxTimeoutTimer = BL_TIMEOUT_VALUE;			//收到动作预警命令，重置超时定时器
		Led_PowerOn();
		if (CanRX_Proc.Data[0] == 0x01)			//第一个字节为0x01
		{
			SetCurrentLightType(LED_EXTEND_MODE);				//声光报警驱动类型
			LightFlag = CanRX_Proc.Data[0];		//LED驱动工作模式
			RGrayScale = CanRX_Proc.Data[1];
			GGrayScale = CanRX_Proc.Data[2];
			BGrayScale = CanRX_Proc.Data[3];
			u16LightOnTimer = (u16)(CanRX_Proc.Data[4] * 10);
			u16LightOffTimer = (u16)(CanRX_Proc.Data[5] * 10);
			if(CanRX_Proc.Data[6] == 0)
			{
				BeepCaseFlag = 1;		//标准发声逻辑
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
				BeepCaseFlag = 0x11;		//可配置发声逻辑
				u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
				u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
			}
		}
		else if(CanRX_Proc.Data[0] == 0x02)		//流水灯效果
		{
			SetCurrentLightType(LED_EXTEND_MODE);				//声光报警驱动类型
			LightFlag = CanRX_Proc.Data[0];		//LED驱动工作模式
			RGrayScale = CanRX_Proc.Data[1];
			GGrayScale = CanRX_Proc.Data[2];
			BGrayScale = CanRX_Proc.Data[3];
			LED_NUM = ((CanRX_Proc.Data[4] & 0xf0) >> 4);
			WaterFlow_Type = CanRX_Proc.Data[4] & 0x0f;
			u16LightOnTimer = (u16)(CanRX_Proc.Data[5] * 10);
			if(CanRX_Proc.Data[6] == 0)
			{
				BeepCaseFlag = 1;		//标准发声逻辑
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
				BeepCaseFlag = 0x11;		//可配置发声逻辑
				u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
				u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
			}
		}
		else if(CanRX_Proc.Data[0] == 0x03)		//呼吸灯效果
		{
			SetCurrentLightType(LED_EXTEND_MODE);				//声光报警驱动类型
			LightFlag = CanRX_Proc.Data[0];		//LED驱动工作模式
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
				BeepCaseFlag = 1;		//标准发声逻辑
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
				BeepCaseFlag = 0x11;		//可配置发声逻辑
				u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
				u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
			}
		}
		else if(CanRX_Proc.Data[0] == 0x04)			//流星灯效果
		{
			SetCurrentLightType(LED_EXTEND_MODE);				//声光报警驱动类型
			LightFlag = CanRX_Proc.Data[0];		//LED驱动工作模式
			RGrayScale = CanRX_Proc.Data[1];
			GGrayScale = CanRX_Proc.Data[2];
			BGrayScale = CanRX_Proc.Data[3];
			LED_NUM = CanRX_Proc.Data[4];
			u16LightOnTimer = (u16)(CanRX_Proc.Data[5] * 10);
			if(CanRX_Proc.Data[6] == 0)
			{
				BeepCaseFlag = 1;		//标准发声逻辑
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
				BeepCaseFlag = 0x11;		//可配置发声逻辑
				u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
				u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
			}
		}
		else if(CanRX_Proc.Data[0] == 0x05)			//多彩灯效果
		{
			SetCurrentLightType(LED_EXTEND_MODE);				//声光报警驱动类型
			LightFlag = CanRX_Proc.Data[0];		//LED驱动工作模式
			RGrayScale = CanRX_Proc.Data[1];
			GGrayScale = CanRX_Proc.Data[1];
			BGrayScale = CanRX_Proc.Data[1];
			LED_NUM = (CanRX_Proc.Data[2] & 0x3f);
			u16LightOnTimer = (u16)(CanRX_Proc.Data[3] * 10);
			if(CanRX_Proc.Data[6] == 0)
			{
				BeepCaseFlag = 1;		//标准发声逻辑
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
				BeepCaseFlag = 0x11;		//可配置发声逻辑
				u16BeepOnTimer = (u16)(((CanRX_Proc.Data[6] & 0xf0) >> 4) * 100);
				u16BeepOffTimer =(u16)((CanRX_Proc.Data[6] & 0x0f) * 100);
			}
		}
		
		
		else
		{
			;
		}
	}
		/**发送给红外接收的数据**/
		#if 1
		//使能433无线模块功能
		if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
		{
			if (tmp.ID.RID == WL_RID)				//数据帧
			{
				switch (tmp.ID.FT)
				{
					case FT_SC_TO_WL_DBUS:							//2				// SC 发给 WL 的总线数据，需要应答
						CanRcvBusProc(&IRCanRX_Proc);
					break;
					case FT_WL_TO_SC_DBUS:							//3				// WL 发给 SC 的总线数据，需要应答
						
					break;
					case FT_SC_TO_WL_IR_MATCH_RST:					//5				// SC 发给 WL 的红外对码结果，需要应答
						CanRcvIrMatchProc(&IRCanRX_Proc);
					break;
					
					case FT_SC_TO_WL_RF_MATCH_RST:					//7				// SC 发给 WL 的无线对码结果，需要应答
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
					break;
					case FT_SC_TO_WL_MATCH_SC_GROUP_INFO:			//8				// SC发送给WL，对码架发送成组信息	
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
					break;
					case FT_SC_TO_WL_UC_SC_GROUP_INFO:				//9				// SC发送给WL，被控架发送成组信息
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
					break;
					case FT_SC_TO_WL_CTL_DATA_RECEPT:				//11			// SC 发送给 遥控器的控制数据是否接收，需要应答			
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
					break;
					case FT_SC_TO_WL_DISCONNECT:					//14			// SC 发送给 WL 解除对码
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
					break;
					case FT_SC_TO_WL_RESET_PAR:						//16			// SC 发送给 WL 重设参数
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[5]);
					break;
					case FT_SC_TO_WL_LIFT_RECEPT:					//17			// SC 发送给 WL 按键抬起是否接受
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[6]);
					break;
					case FT_SC_TO_WL_RESET_PAR_WL:					//22			// 设置无线红外接收模块参数
						CanRcvSetPar(&IRCanRX_Proc);						//PT_SER_SC_NO;
					break;
				    case FT_WL_TO_SC_AUTO_PRESS_CLOSE:					//20			// SC 发送给 WL 按键抬起是否接受
						CanRcvWlSendProc(&IRCanRX_Proc, IRCanRX_Proc.u8Data[0]);
					break;	
//					case FT_SC_TO_WL_MATCH_END:						//31			// 对码直接结束
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
	else if (tmp.ID.RD == RD_PRG_FRAME)					//程序更新帧
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
				//使能角度功能
				if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
				{
					/**挂起不必要的任务**/
					OSTaskQuery(ANGLE_SENSOR_TASK_PRIO, &pdata);			//查询处理任务是否挂起	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(ANGLE_SENSOR_TASK_PRIO);
					}
				}
				//使能UWB人员定位功能
				if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
				{
					/**挂起不必要的任务**/
					OSTaskQuery(UWBAPP_TASK_PRIO, &pdata);			//查询处理任务是否挂起	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{
						OSTaskSuspend(UWBAPP_TASK_PRIO);
					}
					/**挂起不必要的任务**/
					OSTaskQuery(CANTX_TASK_PRIO, &pdata);			//查询处理任务是否挂起	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(CANTX_TASK_PRIO);
					}
				}
				//使能433无线模块功能
				if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
				{
					/**挂起不必要的任务**/
					OSTaskQuery(WL_MANAGE_TASK_PRIO, &pdata);			//查询处理任务是否挂起	
					if(pdata.OSTCBStat != OS_STAT_SUSPEND)
					{	
						OSTaskSuspend(WL_MANAGE_TASK_PRIO);
					}
				}
			}
		}
		//收到传输的程序帧时的处理
		CanRecvProgProc(&stFrame);
	}
	else
	{
		return;
	}
}

/*******************************************************************************************
**函数名称：CanRxProc
**输　入：None
** 输　出：None
** 功能描述：Can发送正常时接收数据处理
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
**函数名称：void CanRx_Task(void *p_arg)
**输　入：None
** 输　出：None
** 功能描述：Can recevie task
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
** 函数名称: 
** 功能描述: 初始化设备所需的数据结构
** 参数描述：CAN APP 初始化
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
	if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)		//使能人员定位功能
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

