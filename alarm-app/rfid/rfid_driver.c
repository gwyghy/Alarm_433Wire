/*********************************************************************************************************************************
** �ļ���:  rfid_driver.c
** �衡��:  RFIDģ������ṩ�Ľӿ�
** ������: 	����
** �ա���:  2014-12-26
** �޸���:	
** �ա���:	
**
** �桡��:	V1.0.0.0
** ���¼�¼:
** ���¼�¼	��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
**--------------------------------------------------------------------------
**************************Copyright (c) 1998-1999 ��������Ӽ������޹�˾����������*************************************************/


#include "includes.h"

#include "can_app.h"

extern RCVED_BACK_CALL_FUNC RcvedBackCallFunc;			// ���պ�Ӧ�ò�ʵ�ֵĻص�������������rfid_cc1101.c
extern u32 SystemID;
extern uint16_t gWlAddr;                    //����ģ��ĵ�ַ
extern WL_WORK_STATUS eCanWLReportMode;     //����ģ��Ĺ���ģʽ

void RfidSpiInit(void);
u32 RFID_FetchData(st_RFIDRcvFrame *RfidRcvFrm);
//----------------------------------------------------------------------------------------------------
// �����ṩ�ĺ����ӿ�
//----------------------------------------------------------------------------------------------------
/*****************************************************************
** ��������SetRcvedBackCallFunc
** �䡡�룺BackCallFunc�����ݽ��պ�Ļص�����
** �䡡������
** ��������������RFID���ݽ��պ�Ļص�����
******************************************************************/
void SetRcvedBackCallFunc(RCVED_BACK_CALL_FUNC BackCallFunc)
{
	RcvedBackCallFunc = BackCallFunc;
}
/***********************************************************************************************
** �� �� ����	RFID_Init
** ����������	�����ݴ洢����ȡ������
** �䡡  �룺	void;
** �䡡  ����	st_RFIDRcvFrame��һ������
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void RFID_Init(void)
{
	RfidSpiInit();
	RFID_HardwareInit();
	RfidGDOxIntInit();

//	SetWlAddr(1);//��������ģ���ַ
	
	//���ý���ģʽ
	SetRxMode();
}
/********************************************************************************
**�������ƣ�RfidSpiInit
**�������ã�����RFID SPI���ߵ�Ӳ���ӿڣ�ʱ�ӡ��ܽţ���Ӳ������
**������������
**�����������
**ע�������
*********************************************************************************/
void RfidSpiInit(void)
{
	#if 0
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	
	/********����ʱ��********/
	RCC_RFID_APBxCmd();//ʹ��SPIʱ��
	RCC_APB2PeriphClockCmd(RCC_RFID_SCLK|RCC_RFID_MOSI|RCC_RFID_MIS0|RCC_RFID_CS|RCC_RFID_GDO, ENABLE); 
	//RCC_RFID_AF_APBxCmd();//�ڳ��������ģ���Ѿ�����AFIO���ù���
	
	
	/*******����˿�����*******/
	//SCLK  MISO  MOSI  	SPI�ڸ��ù��ܶ���
	//GPIO_PinAFConfig(GPIO_RFID_SCLK, GPIO_RFID_PINSOURCE_SCLK, GPIO_RFID_AF_DEFINE);
	//GPIO_PinAFConfig(GPIO_RFID_MISO, GPIO_RFID_PINSOURCE_MISO, GPIO_RFID_AF_DEFINE);
	//GPIO_PinAFConfig(GPIO_RFID_MOSI, GPIO_RFID_PINSOURCE_MOSI, GPIO_RFID_AF_DEFINE);


	/* SCLK����ģʽ�����������������ģʽ���������� */
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

	GPIO_SetBits(GPIO_RFID_CS, PIN_RFID_CS);//cs ��ѡ��

	 /**GDO0**//**����Ŀ��ʹ�ô�������Ϊ�ⲿ�ж�****/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;			//��������	
	GPIO_InitStructure.GPIO_Pin = PIN_RFID_GDO0;			//ѡ��ܽ�
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_RFID_GDO0, &GPIO_InitStructure);			//��ʼ��		

	 /**GDO2**//**����Ŀ��ʹ�ô�������Ϊ�ⲿ�ж�****/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;			//��������	
	GPIO_InitStructure.GPIO_Pin = PIN_RFID_GDO2;			//ѡ��ܽ�
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_RFID_GDO2, &GPIO_InitStructure);			//��ʼ��		

	SPI_I2S_DeInit(RFID_SPI);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;   //ȫ˫��
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;  //8λ����ģʽ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;         //����ģʽ��SCKΪ0
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;       //���ݲ����ӵ�1��ʱ����ؿ�ʼ
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;          //NSS�������
	SPI_InitStructure.SPI_BaudRatePrescaler = RFID_SPI_BAUNDRATE_PRESCALER;  //���÷�Ƶϵ��
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //���ģʽ
	SPI_InitStructure.SPI_CRCPolynomial = 7;           //CRC����ʽ
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;      //����ģʽ
	SPI_Init(RFID_SPI, &SPI_InitStructure);

	SPI_Cmd(RFID_SPI, DISABLE);	                       //�Ƚ�ֹSPI1	
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
	/*Configure CS GPIO pin : spi Ƭѡ */
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
    spi_crc_polynomial_set(SPI2, 7);			//Ĭ��ֵΪ0x0007h��
    /* enable SPI2 */
    spi_enable(SPI2);
}
/********************************************************************************
**�������ƣ�RfidGDOxIntInit
**�������ã�����RFID���ж�
**������������
**�����������
**ע�������
*********************************************************************************/
void RfidGDOxIntInit(void)
{
	#if 0
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	/**��ʼ��INT��Ӧ���ⲿ�ж�****/
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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0f;//��ʹ���ж����ȼ�Ƕ�ס���ΪSysTick���ж����ȼ�Ϊ0x0f //1; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;//0������SysTick //0;        //��Ӧ���ȼ�Ϊ 0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);  
	#endif

    rcu_periph_clock_enable(RCU_GPIOD);//GPIOAʱ��ʹ��
    rcu_periph_clock_enable(RCU_GPIOB);//GPIOAʱ��ʹ��
	
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
** ��������WL_ReceiveData
** �䡡�룺buf�����ݴ��뻺����
**         num�����д���ֽ���
** �䡡����ʵ�ʶ��������ֽ���
** ������������ȡ���յ���������
******************************************************************/
u32 WL_ReceiveData(unsigned char *buf, unsigned int num)
{
	st_RFIDRcvFrame RxFrm;

	if (RFID_GetFrame(&RxFrm) == RET_OK)
	{
//		SetLedState(LED_CAN_RCV,TRUE,3);
#if 0	// for test �����ط�һ��֡
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
//			&& ((RxFrm.u8DestAddr == GET_PT_U16(PT_SER_SC_NO)) 		//��������ַ�������ܱ��
			&& ((RxFrm.u8DestAddr == GetWLMId()) 		//��������ַ�������ܱ��
//			|| (RxFrm.u8DestAddr == WL_SS_ADDR)												// ���������ߵ�ַ(510��Ӧ0xFF)
			|| (RxFrm.u8DestAddr == WL_BROADCAST_ADDR && SystemID == 0)))					//���ȹ���
		{
			u32 i = RxFrm.u8DataLen;		//���ݳ���
			u8 *pt = RxFrm.u8Data;
			u8RssiSignalInf = RxFrm.u8AppendStatus[0];
#if 0	// for test
			if (i > num)
				i = num;
#endif
			if (i > 0)
			{
				memmove(buf, pt, i);	//ȡ����
				return (i);
			}
		}
	}
	
	return(0);
}
/*****************************************************************
** ��������WL_SendData
** �䡡�룺buf���������ݻ�����
**         num�����������ֽ���
** �䡡����ʵ�ʷ����ֽ���
** �������������߷�������
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
** ���ܣ�����Ƿ��������߷���
** ���룺��
** ���أ�TRUE:���Է���
******************************************************************/
u32 WlEmitEnabled(void)
{
	return (TRUE);
}
/***********************************************************************************************
** �� �� ����	RFID_FetchData
** ����������	�����ݴ洢����ȡ������
** �䡡  �룺	void;
** �䡡  ����	st_RFIDRcvFrame��һ������
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 RFID_FetchData(st_RFIDRcvFrame *RfidRcvFrm)
{
	return (RFID_GetFrame(RfidRcvFrm));
}
/***********************************************************************************************
** �� �� ����	RFID_SendData
** ����������	��FIFO��д�����ݣ�
** �䡡  �룺	u8DestAddr,���ݵĽ��յ�ַ��
**				pu8Buf�����ݻ���ָ�룻
**				u32Length�����ݳ���ָ�룻
** �䡡  ����	
** �� �� ֵ��	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
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
	
	// ���뷢�ͺ���
	u8Ret = RFTxSendPacket(pu8DataTmp, u8Length);
	
	//SetRxMode();			//�������ģʽ
	
	return((u32)u8Ret);
	
}
/***********************************************************************************************
** �� �� ����	SetRfidSIDLE
** ����������	����CC1101�������״̬
** �䡡  �룺	��
** �䡡  ����	
** �� �� ֵ��	����״̬
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 SetRfidSIDLE(void)
{
	return(RFCtrlSetIDLE());
}
/***********************************************************************************************
** �� �� ����	SetRfidSRX
** ����������	����CC1101�������״̬
** �䡡  �룺	��
** �䡡  ����	
** �� �� ֵ��	����״̬
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 SetRfidSRX(void)
{
	SetRxMode();
	return 0;
}
/***********************************************************************************************
** �� �� ����	SetWlAddr()
** ����������	����CC1101������ַ
** �䡡  �룺	��
** �䡡  ����	
** �� �� ֵ��	����״̬
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 SetWlAddr(u16 addr)
{
	gWlAddr = addr;
    eCanWLReportMode = WL_NORMAL_STATE;        //����Ϊ����ģʽ
	return(RfidWriteReg(CC1101_ADDR,gWlAddr)); // ����������ַ	
}

/***********************************************************************************************
** �� �� ����	GetRfidCurStatus
** ����������	��ȡCC1101��ǰ״̬
** �䡡  �룺	��
** �䡡  ����	
** �� �� ֵ��	����״̬
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 GetRfidCurStatus(void)
{
	return(RfidGetTxStatus());
}
/*********************************������������޹�˾*************************************************************/
