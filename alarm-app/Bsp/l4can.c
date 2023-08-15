/*! ----------------------------------------------------------------------------
 * @file	l4can.c
 * @brief	stm32l452 can 驱动
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */
#include "l4can.h"
#include "can_bus.h"
#if STM32 
HAL_StatusTypeDef     gCanState;

CAN_HandleTypeDef     CanHandle;
CAN_TxHeaderTypeDef   TxHeader;
CAN_RxHeaderTypeDef   RxHeader;
#endif

can_receive_message_struct RxHeader;

can_trasnmit_message_struct TxHeader;

uint8_t               TxData[8];
uint8_t               RxData[8];
uint32_t              TxMailbox;

uint32_t              gTclk;

void CanError_Handler(void)
{
	#if STM32 
	HAL_CAN_DeInit(&CanHandle);
	#endif
	can_deinit(CAN0);
	NVIC_SystemReset();
}

/*******************************************************************************************
**函数名称：CanTrsDummy
**函数作用：发送一个dummy帧，以规避0#邮箱发中断不再产生的可能。
**函数参数：CAN类型的数据
**函数输出：无
**注意事项：无
*******************************************************************************************/
void CanTrsDummy()
{
	can_trasnmit_message_struct TxCan;
	
	TxCan.tx_ff    = CAN_FF_STANDARD;
	TxCan.tx_ft    = CAN_FT_DATA;
	TxCan.tx_sfid = 0x400;
	TxCan.tx_dlen = 0x00;
	can_message_transmit(CAN0, &TxCan);
}

/*******************************************************************************************
* 函数名称：void CanGpioInit(void)
* 功能描述：can gpio 初始化
* 入口参数：无
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void CanGpioInit(void)
{
	#if STM32 
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*1.时钟使能*/
	CANx_CLK_ENABLE();
	CANx_GPIO_CLK_ENABLE();

	/*2.gpio配置*/
	GPIO_InitStruct.Pin = CANx_TX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  CANx_TX_AF;

	HAL_GPIO_Init(CANx_TX_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CANx_RX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  CANx_RX_AF;

	HAL_GPIO_Init(CANx_RX_GPIO_PORT, &GPIO_InitStruct);

	/*配置NVIC*/
	HAL_NVIC_SetPriority(CANx_RX_IRQn, CAN_RX_IRQ_PRIO, 0);
	HAL_NVIC_EnableIRQ(CANx_RX_IRQn);

	HAL_NVIC_SetPriority(CAN1_SCE_IRQn, CAN_RX_IRQ_PRIO, 0);
	HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
	#endif
    /* enable can clock */
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOA);
    
    /* configure CAN0 GPIO, CAN0_TX(PD1) and CAN0_RX(PD0) */
    gpio_init(CANx_TX_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, CANx_TX_PIN);
    gpio_init(CANx_RX_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, CANx_RX_PIN);
    
//    gpio_pin_remap_config(GPIO_CAN0_FULL_REMAP, ENABLE);			//原生CAN0，无需REMAP

	/* configure CAN0 NVIC */
    nvic_irq_enable(CAN0_RX0_IRQn, CAN_RX_IRQ_PRIO, 0);
	nvic_irq_enable(CAN0_EWMC_IRQn, CAN_RX_IRQ_PRIO, 0);

}

/*******************************************************************************************
* 函数名称：CAN_Config
* 功能描述：can 通信速率配置
* 入口参数：无
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void CAN_Config(uint32_t speed)
{
#if STM32 
	CAN_FilterTypeDef  sFilterConfig;

	/*##-1- Configure the CAN peripheral #######################################*/
	CanHandle.Instance = CANx;

	CanHandle.Init.TimeTriggeredMode = DISABLE;
	CanHandle.Init.AutoBusOff = DISABLE;
	CanHandle.Init.AutoWakeUp = ENABLE;
	CanHandle.Init.AutoRetransmission = ENABLE;
	CanHandle.Init.ReceiveFifoLocked = DISABLE;
	CanHandle.Init.TransmitFifoPriority = ENABLE;
	CanHandle.Init.Mode = CAN_MODE_NORMAL;
	CanHandle.Init.SyncJumpWidth = CAN_SJW_1TQ;
	CanHandle.Init.TimeSeg1 = CAN_BS1_3TQ;
	CanHandle.Init.TimeSeg2 = CAN_BS2_2TQ;
	
	gTclk = HAL_RCC_GetPCLK1Freq();				//系统PCLK1时钟
	CanHandle.Init.Prescaler = gTclk / 1000 / speed / (1 + 3 + 2);

	if (HAL_CAN_Init(&CanHandle) != HAL_OK)
	{
		/* Initialization Error */
		CanError_Handler();
	}

	/*##-2- Configure the CAN Filter ###########################################*/
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&CanHandle, &sFilterConfig) != HAL_OK)
	{
		/* Filter configuration Error */
		CanError_Handler();
	}

	/*##-3- Start the CAN peripheral ###########################################*/
	if (HAL_CAN_Start(&CanHandle) != HAL_OK)
	{
		/* Start Error */
		CanError_Handler();
	}

	/*##-4- Activate CAN RX notification #######################################*/
	if (HAL_CAN_ActivateNotification(&CanHandle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		/* Notification Error */
		CanError_Handler();
	}
	
	/*##-4- Activate CAN RX notification #######################################*/
	if (HAL_CAN_ActivateNotification(&CanHandle, CAN_IT_LAST_ERROR_CODE|CAN_IT_ERROR) != HAL_OK)
	{
		/* Notification Error */
		CanError_Handler();
	}
#endif

	can_parameter_struct            can_parameter;
    can_filter_parameter_struct     can_filter;
	
    gTclk = rcu_clock_freq_get(CK_APB1);
		
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
    
    /* initialize CAN register */
    can_deinit(CAN0);
    
    /* initialize CAN */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = ENABLE;
    can_parameter.no_auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_3TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_2TQ;
//    /* baudrate 200kbps */
	gTclk = rcu_clock_freq_get(CK_APB1);
	/* baudrate 200kbps */
	can_parameter.prescaler = gTclk / 1000 / speed / (3 + can_parameter.time_segment_1 + can_parameter.time_segment_2);
		
    can_init(CAN0, &can_parameter);

    /* initialize filter */

    /* CAN0 filter number */
    can_filter.filter_number = 0;
    /* initialize filter */    
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;  
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
    can_filter_init(&can_filter);
	/* enable CAN receive FIFO0 not empty interrupt */
    can_interrupt_enable(CAN0, CAN_INTEN_RFNEIE0 | CAN_INTEN_RFNEIE1);

	CanTrsDummy();
}

#if INVOKED		//声明实际未被调用
/*******************************************************************************************
* 函数名称：void StartCanRx(void)
* 功能描述：
* 入口参数：无
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void StartCanRx(void)
{
//	uint32_t interrupts = READ_REG(hcan->Instance->IER);
	if (HAL_CAN_ActivateNotification(&CanHandle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		/* Notification Error */
		CanError_Handler();
	}
}
#endif

/*******************************************************************************************
* 函数名称：CAN0_RX0_IRQHandler
* 功能描述：can 数据中断接收服务程序。
* 入口参数：无
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void CAN0_RX0_IRQHandler(void)
{
	#if STM32 
	HAL_CAN_IRQHandler(&CanHandle);

	/* Get RX message */
//	CAN_BusRxDataCallback(0, RxHeader, RxData);   //此为can1的回调函数
	#endif

//	u8 framenum;

	if(can_receive_message_length_get(CAN0, CAN_FIFO0) != 0)
	{
		can_message_receive(CAN0, CAN_FIFO0, &RxHeader);		//接收CAN数据帧
		memcpy(RxData, RxHeader.rx_data, RxHeader.rx_dlen);
		CAN_BusRxDataCallback(0, RxHeader, RxData);				//放入接收缓存
//		can_fifo_release(CAN0, CAN_FIFO0);		                 //打开接收缓冲区0，可以接收新数据
	}
	if(can_receive_message_length_get(CAN0, CAN_FIFO1) != 0)
	{
		can_message_receive(CAN0, CAN_FIFO1, &RxHeader);
		memcpy(RxData, RxHeader.rx_data, RxHeader.rx_dlen);
		CAN_BusRxDataCallback(0, RxHeader, RxData);
//		can_fifo_release(CAN0,CAN_FIFO1);		                 //打开接收缓冲区1，可以接收新数据
	}

}

void CANx_Err_IRQHandler(void)
{
	#if STM32 
	HAL_CAN_DeInit(&CanHandle);
	#endif
	can_deinit(CAN0);
	NVIC_SystemReset();		
//	HAL_CAN_IRQHandler(&CanHandle);
}

#if STM32 
__weak void CAN_BusRxDataCallback(uint16_t canid, CAN_RxHeaderTypeDef head, uint8_t * data)
{
	 UNUSED(canid);
	 UNUSED(head);
	 UNUSED(data);
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	/* Get RX message */
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
	{
		/* Reception Error */
		CanError_Handler();
	}
	else
	{
		CAN_BusRxDataCallback(0, RxHeader, RxData);   //此为can1的回调函数
	}

}
#endif

/*********************************************************

************************************************************/
int32_t STM32_CAN_Write(uint16_t canid, sCAN_FRAME txMessage, uint16_t frametype)
{
	if(frametype == CAN_FF_STANDARD)
		TxHeader.tx_sfid = txMessage.Stdid;
	else
		TxHeader.tx_efid = txMessage.Stdid;
	TxHeader.tx_ff = frametype;
	TxHeader.tx_dlen = txMessage.DLC;
	memcpy(TxHeader.tx_data, txMessage.Data, txMessage.DLC);
	
#if STM32 
	gCanState = HAL_CAN_AddTxMessage(&CanHandle, &TxHeader, TxData, &TxMailbox);
	if (gCanState != HAL_OK)
	{
		/* Reception Error */
		if(CanHandle.ErrorCode == HAL_CAN_ERROR_PARAM)
			return 1;
		CanError_Handler();
	}
	//return HAL_CAN_AddTxMessage(&CanHandle, &TxHeader, TxData, &TxMailbox);
#endif
	
	TxMailbox = can_message_transmit(CAN0, &TxHeader);
//	if (TxMailbox == CAN_NOMAILBOX)
//	{
//		/* Reception Error */
//			return 1;
//		CanError_Handler();
//	}
	return 0;
}

