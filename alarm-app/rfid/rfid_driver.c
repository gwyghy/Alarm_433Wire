/*********************************************************************************************************************************
** 文件名:  rfid_driver.c
** 描　述:  RFID模块对外提供的接口
** 创建人: 	沈万江
** 日　期:  2014-12-26
** 修改人:	
** 日　期:	
**
** 版　本:	V1.0.0.0
** 更新记录:
** 更新记录	：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
**--------------------------------------------------------------------------
**************************Copyright (c) 1998-1999 天津华宁电子技术有限公司技术开发部*************************************************/


#include "includes.h"

#include "can_app.h"

extern RCVED_BACK_CALL_FUNC RcvedBackCallFunc;			// 接收后应用层实现的回调函数。定义在rfid_cc1101.c
extern u32 SystemID;
extern uint16_t gWlAddr;                    //无线模块的地址
extern WL_WORK_STATUS eCanWLReportMode;     //无线模块的工作模式

void RfidSpiInit(void);
u32 RFID_FetchData(st_RFIDRcvFrame *RfidRcvFrm);
//----------------------------------------------------------------------------------------------------
// 对外提供的函数接口
//----------------------------------------------------------------------------------------------------
/*****************************************************************
** 函数名：SetRcvedBackCallFunc
** 输　入：BackCallFunc：数据接收后的回调函数
** 输　出：无
** 功能描述：设置RFID数据接收后的回调函数
******************************************************************/
void SetRcvedBackCallFunc(RCVED_BACK_CALL_FUNC BackCallFunc)
{
	RcvedBackCallFunc = BackCallFunc;
}
/***********************************************************************************************
** 函 数 名：	RFID_Init
** 功能描述：	从数据存储池中取出数据
** 输　  入：	void;
** 输　  出：	st_RFIDRcvFrame：一包数据
** 作　  者：	沈万江
** 日　  期：	2014.12.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
void RFID_Init(void)
{
	RfidSpiInit();
	RFID_HardwareInit();
	RfidGDOxIntInit();

//	SetWlAddr(1);//设置无线模块地址
	
	//设置接收模式
	SetRxMode();
}
/********************************************************************************
**函数名称：RfidSpiInit
**函数作用：配置RFID SPI总线的硬件接口（时钟、管脚）和硬件参数
**函数参数：无
**函数输出：无
**注意事项：无
*********************************************************************************/
void RfidSpiInit(void)
{
	#if 0
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	/********配置时钟********/
	RCC_RFID_APBxCmd();//使能SPI时钟
	RCC_APB2PeriphClockCmd(RCC_RFID_SCLK|RCC_RFID_MOSI|RCC_RFID_MIS0|RCC_RFID_CS|RCC_RFID_GDO, ENABLE); 
	//RCC_RFID_AF_APBxCmd();//在程序的其它模块已经打开了AFIO复用功能
	
	
	/*******输出端口配置*******/
	//SCLK  MISO  MOSI  	SPI口复用功能定义
	//GPIO_PinAFConfig(GPIO_RFID_SCLK, GPIO_RFID_PINSOURCE_SCLK, GPIO_RFID_AF_DEFINE);
	//GPIO_PinAFConfig(GPIO_RFID_MISO, GPIO_RFID_PINSOURCE_MISO, GPIO_RFID_AF_DEFINE);
	//GPIO_PinAFConfig(GPIO_RFID_MOSI, GPIO_RFID_PINSOURCE_MOSI, GPIO_RFID_AF_DEFINE);


	/* SCLK：主模式，复用推免输出；从模式，浮空输入 */
	GPIO_InitStructure.GPIO_Pin = RFID_SPI_MCU_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIO_RFID_SCLK, &GPIO_InitStructure);

	/**MIS0***/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = PIN_RFID_MISO;
	GPIO_Init(GPIO_RFID_MISO, &GPIO_InitStructure);

	/**MOSI***/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = PIN_RFID_MOSI;
	GPIO_Init(GPIO_RFID_MOSI, &GPIO_InitStructure);	

	/*rfid-CS*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = PIN_RFID_CS;	
	GPIO_Init(GPIO_RFID_CS, &GPIO_InitStructure);

	GPIO_SetBits(GPIO_RFID_CS, PIN_RFID_CS);//cs 不选中

	 /**GDO0**//**本项目中使用此引脚作为外部中断****/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;			//悬空输入	
	GPIO_InitStructure.GPIO_Pin = PIN_RFID_GDO0;			//选择管脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_RFID_GDO0, &GPIO_InitStructure);			//初始化		

	 /**GDO2**//**本项目中使用此引脚作为外部中断****/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;			//悬空输入	
	GPIO_InitStructure.GPIO_Pin = PIN_RFID_GDO2;			//选择管脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_RFID_GDO2, &GPIO_InitStructure);			//初始化		

	SPI_I2S_DeInit(RFID_SPI);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;   //全双工
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;  //8位数据模式
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;         //空闲模式下SCK为0
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;       //数据采样从第1个时间边沿开始
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;          //NSS软件管理
	SPI_InitStructure.SPI_BaudRatePrescaler = RFID_SPI_BAUNDRATE_PRESCALER;  //设置分频系数
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //大端模式
	SPI_InitStructure.SPI_CRCPolynomial = 7;           //CRC多项式
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;      //主机模式
	SPI_Init(RFID_SPI, &SPI_InitStructure);

	SPI_Cmd(RFID_SPI, DISABLE);	                       //先禁止SPI1	
	SPI_Cmd(RFID_SPI, ENABLE);
	#endif

	spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_SPI2);
	rcu_periph_clock_enable(RCU_AF);
	
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
//	gpio_pin_remap_config(GPIO_SWJ_NONJTRST_REMAP, ENABLE);
	/**SPI2 GPIO Configuration
	PC10 ------> SPI2_SCK_PIN
	PC11 ------> SPI2_MISO_PIN
	PC12 ------> SPI2_MOSI_PIN
	**************************/
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, PIN_RFID_SCLK);
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, PIN_RFID_MISO);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, PIN_RFID_MOSI);
	/*Configure CS GPIO pin : spi 片选 */
	gpio_init(GPIO_RFID_CS, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, PIN_RFID_CS);		//PA15

	gpio_pin_remap_config(GPIO_SPI2_REMAP, ENABLE);
	
    spi_i2s_deinit(SPI2);
    spi_struct_para_init(&spi_init_struct);
    /* SPI0 parameter config */
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = RFID_SPI_BAUNDRATE_PRESCALER;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI2, &spi_init_struct);

    /* set crc polynomial */
    spi_crc_polynomial_set(SPI2, 7);			//默认值为0x0007h。
    /* enable SPI2 */
    spi_enable(SPI2);
}
/********************************************************************************
**函数名称：RfidGDOxIntInit
**函数作用：配置RFID的中断
**函数参数：无
**函数输出：无
**注意事项：无
*********************************************************************************/
void RfidGDOxIntInit(void)
{
	#if 0
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	/**初始化INT对应的外部中断****/
	/* Connect EXTI Line to GDO0 Pin */
	GPIO_EXTILineConfig(RFID_CC1101_GDOx_PORT_SOURCE, RFID_CC1101_GDO0_PIN_SOURCE);//GPIO_PortSourceGPIOC
	/* Connect EXTI Line to GDO2 Pin */
	GPIO_EXTILineConfig(RFID_CC1101_GDOx_PORT_SOURCE, RFID_CC1101_GDO2_PIN_SOURCE);//GPIO_PortSourceGPIOC

	/* Configure GDO0 EXTI line */
	EXTI_InitStructure.EXTI_Line    = RFID_GDO0_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Configure GDO2 EXTI line */
	EXTI_InitStructure.EXTI_Line    = RFID_GDO2_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_ClearFlag(RFID_GDO2_EXTI_LINE);	
	EXTI_ClearFlag(RFID_GDO0_EXTI_LINE);

	/* Enable and set the EXTI interrupt to priority 1*/
	NVIC_InitStructure.NVIC_IRQChannel = RFID_GDOx_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0f;//不使用中断优先级嵌套。因为SysTick的中断优先级为0x0f //1; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;//0级用于SysTick //0;        //响应优先级为 0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);  
	#endif

    rcu_periph_clock_enable(RCU_GPIOD);//GPIOA时钟使能
    rcu_periph_clock_enable(RCU_GPIOB);//GPIOA时钟使能
	
	gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
	gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* connect key EXTI line to key GPIO pin */
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_3);
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOD, GPIO_PIN_SOURCE_2);
    /* configure key EXTI line */
	exti_init(EXTI_3, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	nvic_irq_enable(EXTI3_IRQn, 0x0f, 0U);
	exti_interrupt_flag_clear(EXTI_3);
	
	exti_init(EXTI_2, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	nvic_irq_enable(EXTI2_IRQn, 0x0f, 0U);
	exti_interrupt_flag_clear(EXTI_2);

}
u8  u8RssiSignalInf;
/*****************************************************************
** 函数名：WL_ReceiveData
** 输　入：buf：数据存入缓冲区
**         num：最大写入字节数
** 输　出：实际读出数据字节数
** 功能描述：读取接收的无线数据
******************************************************************/
u32 WL_ReceiveData(unsigned char *buf, unsigned int num)
{
	st_RFIDRcvFrame RxFrm;

	if (RFID_GetFrame(&RxFrm) == RET_OK)
	{
//		SetLedState(LED_CAN_RCV,TRUE,3);
#if 0	// for test 立即回发一个帧
		{
			u8	Text[16];
			u32 Len=13;
			Text[0] = 0x31;
			Text[1] = 0x32;
			Text[2] = 0x33;
			Text[3] = 0x34;
			Text[4] = 0x35;
			Text[5] = 0x36;
			Text[6] = 0x37;
			Text[7] = 0x38;
			Text[8] = 0x39;
			Text[9] = 0x30;
			Text[10] = 0x41;
			Text[11] = 0x42;
			Text[12] = 0x43;
			(void)RFID_SendData(0xFE, Text, &Len);
		}
#endif
		if ((num >= RxFrm.u8DataLen) 
//			&& ((RxFrm.u8DestAddr == GET_PT_U16(PT_SER_SC_NO)) 		//控制器地址，即本架编号
			&& ((RxFrm.u8DestAddr == GetWLMId()) 		//控制器地址，即本架编号
//			|| (RxFrm.u8DestAddr == WL_SS_ADDR)												// 服务器无线地址(510对应0xFF)
			|| (RxFrm.u8DestAddr == WL_BROADCAST_ADDR && SystemID == 0)))					//长度过滤
		{
			u32 i = RxFrm.u8DataLen;		//数据长度
			u8 *pt = RxFrm.u8Data;
			u8RssiSignalInf = RxFrm.u8AppendStatus[0];
#if 0	// for test
			if (i > num)
				i = num;
#endif
			if (i > 0)
			{
				memmove(buf, pt, i);	//取数据
				return (i);
			}
		}
	}
	
	return(0);
}
/*****************************************************************
** 函数名：WL_SendData
** 输　入：buf：发送数据缓冲区
**         num：发送数据字节数
** 输　出：实际发送字节数
** 功能描述：无线发射数据
******************************************************************/
u32 WL_SendData(unsigned char dest_addr, unsigned char *buf, unsigned int num)
{
	u32 Len=num;
	
	if (RFID_SendData(dest_addr, buf, &Len) == RET_OK)
		return(num);
	else
		return(0);
}
/*****************************************************************
** 功能：检查是否允许无线发射
** 输入：无
** 返回：TRUE:可以发送
******************************************************************/
u32 WlEmitEnabled(void)
{
	return (TRUE);
}
/***********************************************************************************************
** 函 数 名：	RFID_FetchData
** 功能描述：	从数据存储池中取出数据
** 输　  入：	void;
** 输　  出：	st_RFIDRcvFrame：一包数据
** 作　  者：	沈万江
** 日　  期：	2014.12.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 RFID_FetchData(st_RFIDRcvFrame *RfidRcvFrm)
{
	return (RFID_GetFrame(RfidRcvFrm));
}
/***********************************************************************************************
** 函 数 名：	RFID_SendData
** 功能描述：	向FIFO中写入数据；
** 输　  入：	u8DestAddr,数据的接收地址；
**				pu8Buf，数据缓存指针；
**				u32Length，数据长度指针；
** 输　  出：	
** 返 回 值：	
** 作　  者：	沈万江
** 日　  期：	2014.12.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 RFID_SendData(u8 u8DestAddr,u8 *pu8Buf, u32 *u32Length)
{
	u8 pu8DataTmp[64],u8Length,u8Ret;
	u32 u32i;
	
#if (CC1101_ADDR_FILTER > 0)
	pu8DataTmp[1] = u8DestAddr;
	for(u32i = 2; u32i < (*u32Length + 2); u32i++)
	{
		pu8DataTmp[u32i] = pu8Buf[u32i-2];
	}
	u8Length = (u8)(*u32Length) + 1;
#else
	for(u32i = 1; u32i < (*u32Length + 1); u32i++)
	{
		pu8DataTmp[u32i] = pu8Buf[u32i-1];
	}
	u8Length = (u8)(*u32Length);
#endif
	pu8DataTmp[0] = u8Length;
	
	// 加入发送函数
	u8Ret = RFTxSendPacket(pu8DataTmp, u8Length);
	
	//SetRxMode();			//进入接收模式
	
	return((u32)u8Ret);
	
}
/***********************************************************************************************
** 函 数 名：	SetRfidSIDLE
** 功能描述：	设置CC1101进入空闲状态
** 输　  入：	无
** 输　  出：	
** 返 回 值：	返回状态
** 作　  者：	沈万江
** 日　  期：	2014.12.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 SetRfidSIDLE(void)
{
	return(RFCtrlSetIDLE());
}
/***********************************************************************************************
** 函 数 名：	SetRfidSRX
** 功能描述：	设置CC1101进入接收状态
** 输　  入：	无
** 输　  出：	
** 返 回 值：	返回状态
** 作　  者：	沈万江
** 日　  期：	2014.12.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 SetRfidSRX(void)
{
	SetRxMode();
	return 0;
}
/***********************************************************************************************
** 函 数 名：	SetWlAddr()
** 功能描述：	设置CC1101器件地址
** 输　  入：	无
** 输　  出：	
** 返 回 值：	返回状态
** 作　  者：	沈万江
** 日　  期：	2014.12.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 SetWlAddr(u16 addr)
{
	gWlAddr = addr;
    eCanWLReportMode = WL_NORMAL_STATE;        //设置为正常模式
	return(RfidWriteReg(CC1101_ADDR,gWlAddr)); // 设置器件地址	
}

/***********************************************************************************************
** 函 数 名：	GetRfidCurStatus
** 功能描述：	获取CC1101当前状态
** 输　  入：	无
** 输　  出：	
** 返 回 值：	返回状态
** 作　  者：	沈万江
** 日　  期：	2014.12.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 GetRfidCurStatus(void)
{
	return(RfidGetTxStatus());
}
/*********************************天津华宁电子有限公司*************************************************************/
