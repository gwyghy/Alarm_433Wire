/*********************************************************************************************************************************
** �ļ���:  rfid_cc1101.c
** �衡��:  RFID����ģ��ʵ���ļ�
** 		
**			RFIDģ����ʹ�õ�һЩ��������˵����
**
** 				Chipcon
** 				Product = CC1101
** 				Chip version = A   (VERSION = 0x04)
**				Crystal accuracy = 10 ppm
** 				X-tal frequency = 26 MHz
** 				RF output power = 0 dBm
** 				RX filterbandwidth = 541.666667 kHz
** 				Deviation = 127 kHz
** 				Datarate = 249.938965 kBaud
** 				Modulation = (1) GFSK
** 				Manchester enable = (0) Manchester disabled
** 				RF Frequency = 432.999817 MHz
** 				Channel spacing = 199.951172 kHz
** 				Channel number = 0
** 				Optimization = Sensitivity
** 				Sync mode = (3) 30/32 sync word bits detected
** 				Format of RX/TX data = (0) Normal mode, use FIFOs for RX and TX
** 				CRC operation = (1) CRC calculation in TX and CRC check in RX enabled
** 				Forward Error Correction = (0) FEC disabled
** 				Length configuration = (1) Variable length packets, packet length configured by the first received byte after sync word.
** 				Packetlength = 255
** 				Preamble count = (2)  4 bytes
** 				Append status = 1
** 				Address check = (0) No address check
** 				FIFO autoflush = 0
** 				Device address = 0
** 				GDO0 signal selection = ( 6) Asserts when sync word has been sent / received, and de-asserts at the end of the packet
** 				GDO2 signal selection = (41) CHIP_RDY
**
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

vu32 vu32GDO2IntFlag = 0;	// GDO�����ش�����־������һ�����ݽ���У����ȷ
vu32 vu32GDO0IntFlag = 0;	// GDO�½��ش�����־������һ�����ݷ������
const u8 uc8Patabel[8] = {0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0};

/*const*/ Tag_RF_CONFIG st_RfConfig = {
    0x0C,   // FSCTRL1   	Ƶ�ʺϳ������� 1
    0x00,   // FSCTRL0   	Ƶ�ʺϳ������� 0
    0x10,   // FREQ2     	Ƶ�ʿ�����, ���ֽ�
    0xA7,   // FREQ1     	Ƶ�ʿ�����, �θ��ֽ�
    0x62,   // FREQ0     	Ƶ�ʿ�����, ���ֽ�

    0x2D,   // MDMCFG4   	��������
    0x3B,   // MDMCFG3   	��������
    0x03,   // MDMCFG2   	��������
    0x22,   // MDMCFG1   	��������
    0xF8,   // MDMCFG0   	��������

    0x00,   // CHANNR    	ͨ����
    0x62,   // DEVIATN   	���ƽ����ƫ������ (when FSK modulation is enabled).
    0xB6,   // FREND1    	ǰ��RX ����
    0x10,   // FREND0    	ǰ��TX ����
    0x18,   // MCSM0     	CC1101�����������߼�״̬����

    0x1D,   // FOCCFG    	Ƶ��ƫ�Ʋ�������
    0x1C,   // BSCFG     	λͬ������
    0xC7,   // AGCCTRL2  	AGC ����
    0x00,   // AGCCTRL1  	AGC ����
    0xB0,   // AGCCTRL0  	AGC ����

    0xEA,   // FSCAL3    	Ƶ�ʺϳ���У׼
    0x2A,   // FSCAL2    	Ƶ�ʺϳ���У׼
    0x00,   // FSCAL1    	Ƶ�ʺϳ���У׼
    0x1F,   // FSCAL0    	Ƶ�ʺϳ���У׼
    0x59,   // FSTEST    	Ƶ�ʺϳ���У׼

    0x88,   // TEST2     	��ͬ��������
    0x31,   // TEST1     	��ͬ��������
    0x09,   // TEST0     	��ͬ��������
    0x07,   // IOCFG2    	GDO2 ����ܽ����ã��������ݽ��գ������յ�����У����ȷ��MCUһ���ⲿ�ж��ź�
    0x06,   // IOCFG0    	GDO0 ����ܽ����ã��������ݷ��ͣ���������ɺ�����ж��ź� 
#if (CC1101_ADDR_FILTER > 0)
    0x0E,   // PKTCTRL1  	���ݰ��Զ����ƼĴ���1��0x00001100,
			//				bit3 = 1,CRCУ�鲻��ȷ�Զ���ˢRX FIFO����ʧ��
			//				bit2 = 1,���ݺ����ٸ�������״̬�ֽ�
			//				bit1-0 = 00���޵�ַ��飻 =10,��ַ��飬00Ϊ�㲥��ַ��
#else
	0x0C,
#endif
    0x45,   // PKTCTRL0  	���ݰ��Զ����ƼĴ���0��0x01000101��
			// 				bit6 = 1�����ݼ��ܿ�����
			// 				bit5-4 = 00�����������շ�ģʽ��
			// 				bit2 = 1��CRCʹ�ܣ�
			// 				bit1-0 = 01���ɱ����ݰ�����ģʽ,length ��sync��ߵ� 1 ���ֽڣ�
			
    0xFE,   // ADDR      	������ַ��Ĭ��0x00
    0x40,   // PKTLEN    	���ݰ����ȣ�length = 61;
};
const u8 st_RfConfigIndex[]={
    CC1101_FSCTRL1,     // FSCTRL1   Frequency synthesizer control.
    CC1101_FSCTRL0,     // FSCTRL0   Frequency synthesizer control.
    CC1101_FREQ2,       // FREQ2     Frequency control word, high byte.
    CC1101_FREQ1,       // FREQ1     Frequency control word, middle byte.
    CC1101_FREQ0,       // FREQ0     Frequency control word, low byte.
 
    CC1101_MDMCFG4,     // MDMCFG4   Modem configuration.
    CC1101_MDMCFG3,     // MDMCFG3   Modem configuration.
    CC1101_MDMCFG2,     // MDMCFG2   Modem configuration.
    CC1101_MDMCFG1,     // MDMCFG1   Modem configuration.
    CC1101_MDMCFG0,     // MDMCFG0   Modem configuration.
 
    CC1101_CHANNR,      // CHANNR    Channel number.
    CC1101_DEVIATN,     // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
    CC1101_FREND1,      // FREND1    Front end RX configuration.
    CC1101_FREND0,      // FREND0    Front end TX configuration.
    CC1101_MCSM0,       // MCSM0     Main Radio Control State Machine configuration.
 
    CC1101_FOCCFG,      // FOCCFG    Frequency Offset Compensation Configuration.
    CC1101_BSCFG,       // BSCFG     Bit synchronization Configuration.
    CC1101_AGCCTRL2,    // AGCCTRL2  AGC control.
    CC1101_AGCCTRL1,    // AGCCTRL1  AGC control.
    CC1101_AGCCTRL0,    // AGCCTRL0  AGC control.
 
    CC1101_FSCAL3,      // FSCAL3    Frequency synthesizer calibration.
    CC1101_FSCAL2,      // FSCAL2    Frequency synthesizer calibration.
    CC1101_FSCAL1,      // FSCAL1    Frequency synthesizer calibration.
    CC1101_FSCAL0,      // FSCAL0    Frequency synthesizer calibration.
    CC1101_FSTEST,      // FSTEST    Frequency synthesizer calibration.
 
    CC1101_TEST2,       // TEST2     Various test settings.
    CC1101_TEST1,       // TEST1     Various test settings.
    CC1101_TEST0,       // TEST0     Various test settings.
    CC1101_IOCFG2,      // IOCFG2    GDO2 output pin configuration.
    CC1101_IOCFG0,      // IOCFG0    GDO0 output pin configuration.
 
    CC1101_PKTCTRL1,    // PKTCTRL1  Packet automation control.
    CC1101_PKTCTRL0,    // PKTCTRL0  Packet automation control.
    CC1101_ADDR,        // ADDR      Device address.
    CC1101_PKTLEN,      // PKTLEN    Packet length.
};
//----------------------------------------------------------------------------------------
// 		���鳣������
//----------------------------------------------------------------------------------------
/*
 * Optimum PATABLE Settings for Various Output Power Levels
 */
const u8 s_u8RFOutputPowerCfgTab[8] =
{
/*  PATABLE Setings     Output Power    comment*/
    0x12,               //-30dBm
    0x0E,               //-20dBm
    0x1D,               //-15dBm
    0x34,               //-10dBm
    0x60,               //0dBm
    0x84,               //+5dBm
    0xC8,               //+7dBm
    0xC0,               //+10dBm
};
/*
 * FIFOTHR.CLOSE_IN_RX - RX Attenuation, Typical Values
 */
static const u8 s_u8RFAttenuationCfgTab[4] =
{
/*  FIFOTHR Setings     	CLOSE_IN_RX     RX Attenuation*/
    0x07,                	// 0(00)         	0dB
    0x17,               	// 1(01)         	6dB
    0x27,               	// 2(10)         	12dB
    0x37                	// 3(11)         	18dB        
};
/* 
 * FIFOTHR.ADC_RETENTION - RX FIFO �� TX FIFO ��ֵ
 */
static const u8 s_u8RFCReg_FIFOTHR_BIT6[2] = 
{
    0x00,       			//250KBaud
    0x40        			//38.4KBaud
};
const u8 s_u8RFCReg_MCSM2[2] = 
{
    0x06,       			//250KBaud
    0x04       				//38.4KBaud
};
//Data Rate �Ĵ�������
/* 
 * MDMCFG4 - Modem Configuration: Sets the decimation ratio for 
 * the delta-sigma ADC input stream and thus the channel bandwidth. 
 */
const u8 s_u8RFCReg_MDMCFG4[2] = 
{
    0x2D,       //250KBaud
    0xCA        //38.4KBaud
};
/* MDMCFG3 - Modem Configuration: The mantissa of the user specified symbol rate.*/
const u8 s_u8RFCReg_MDMCFG3[2] = 
{
    0x3B,       //250KBaud
    0x83        //38.4KBaud
};
/* FSCAL3 - Frequency Synthesizer Calibration */
const u8 s_u8RFCReg_FSCAL3[2] = 
{
    0xEA,       //250KBaud
    0xE9        //38.4KBaud
};

/* TEST2 - Various Test Settings */
const u8 s_u8RFCReg_TEST2[2] = 
{
    0x88,       //250KBaud
    0x81        //38.4KBaud
};
/* TEST1 - Various Test Settings */
const u8 s_u8RFCReg_TEST1[2] = 
{
    0x31,       //250KBaud
    0x35        //38.4KBaud
};
/* DEVIATN - Modem Deviation Setting */
const u8 s_u8RFCReg_DEVIATN[2] = 
{
    0x62,       //250KBaud
    0x34        //38.4KBaud
};
u8 g_u8RFVelocityIdx = 1;		//38.4kbps	//0;	//250kbps
u16 g_u16RunningMode = OBUMODE_ONROAD;

Tag_RFChipPar st_RFChipPar = 
{
	DEFAULT_RF_PWRIDX, 
	DEFAULT_RF_ATNIDX, 
	DEFAULT_RF_VELOCITY, 
	DEFAULT_RES_INTERVAL_TIME, 
	DEFAULT_SLOT_TIME, 
	DEFAULT_SLOT_SND_NUM
};
void RfidConfig(/*const*/ Tag_RF_CONFIG* ptRfConfig, const u8 u8RfPaTable);

u8 u8RcvLength = 64;										// ���ڴ洢��CC1101��RX FIFO�н��յ����ݵĳ���
u8 u8RcvData[CC1101_RX_FIFO_SIZE+8];						// ���ڴ洢��CC1101��RX FIFO�н��յ�һ������
#define RFID_RCV_FRM_SIZE			50						// �洢�س���
st_RFIDRcvFrame st_RFIDRcvFrmPool[RFID_RCV_FRM_SIZE];		// RFID�������ݴ洢��
u32 u32RcvFrmPoolWritePtr = 0;
u32 u32RcvFrmPoolReadPtr = 0;
u32 u32RcvFrmPoolCnt = 0;

vu32 vu32CC1100State;										// 1=CC1100���ڷ���״̬
RCVED_BACK_CALL_FUNC RcvedBackCallFunc=NULL;				// ���պ�Ӧ�ò�ʵ�ֵĻص�����

u32 SystemID;
/***********************************************************************************************
** �� �� ����	DelayUs()
** ����������	
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void DelayUs(u32 u32Counter)
{
	u32 u32i;
	u32 u32j;
	u32 uSLoops;
	uint32_t              gTclk;
//    RCC_ClocksTypeDef  rcc_clocks; 
	
    gTclk = rcu_clock_freq_get(CK_SYS);
	
 //   RCC_GetClocksFreq(&rcc_clocks);   //���ñ�׼�⺯������ȡϵͳʱ�ӡ�
//	uSLoops = rcc_clocks.HCLK_Frequency/1000000;
	uSLoops = gTclk / 1000000;
	for (u32i = 0; u32i < u32Counter; u32i++)
	{
 		u32j = 0;
		while(u32j++<uSLoops);
	}
	
// 	//__disable_irq();//��ȫ���ж�
// 	for (u32i = 0; u32i < u32Counter; u32i++)
// 	{
// 		for(u32j = 0; u32j < 10; u32j++)
// 		/* Nothing to do */;
// 	}
// 	//__enable_irq();//��ȫ���ж�
}

/***********************************************************************************************
** �� �� ����	RFID_HardwareInit()
** ����������	
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void RFID_HardwareInit(void)
{
	RfidChipReset();
	
    /*
	 * оƬ����
	 */ 	 
	RfidConfig(&st_RfConfig,s_u8RFOutputPowerCfgTab[st_RFChipPar.u8RFPwrIdx]);

	/*
	 * Additional chip configuration
	 */ 
	RfidWriteReg(CC1101_MCSM1, 0x0F); // No CCA, Stay in RX after RX and TX
	//RfidWriteReg(CC1101_MCSM1, 0x0C); // No CCA, Stay in RX after RX
	//RfidWriteReg(CC1101_MCSM1, 0x30); // CCA enabled, IDLE after TX and RX

	RfidWriteReg(CC1101_SYNC0, 0x33); // SYNC0
	RfidWriteReg(CC1101_SYNC1, 0x33); // SYNC1
	SystemID = 0;		//�������Һ��ϵͳ
	
	RfidWriteReg(CC1101_IOCFG0,0x06);
	RfidWriteReg(CC1101_IOCFG2,0x07);
}

	//���ý���ģʽ
	//SetRxMode();
//}
/***********************************************************************************************
** �� �� ����	SetSync
** �䡡  �룺	SyncWord: ����ͬ���֣����ֽ�=SYNC0�����ֽ�=SYNC1
** ����������	��������ͬ����
************************************************************************************************/
void SetSync(u32 SyncWord)
{
	RfidWriteReg(CC1101_SYNC0, SyncWord&0xff); 		// SYNC0
	RfidWriteReg(CC1101_SYNC1, (SyncWord>>8)&0xff); // SYNC1
	switch (SyncWord)
	{
		case 0x8E57:
			SystemID = 3;		//��������
		break;
		case 0x8E56:
			SystemID = 2;		//��ͷ��ǰ
		break;
		case 0x8E55:
			SystemID = 1;		//��β��ǰ
		break;
		default:
			SystemID = 0;		//�������Һ��ϵͳ
		break;
	}
}
/***********************************************************************************************
** �� �� ����	WaitGPIOReset
** ����������	�ȴ�ָ��GPIO����Reset�ź�
** �䡡  �룺	port: GPIO PORT
**         ��	pin: GPIO PIN
** �䡡  ����	FALSE=�ȴ���ʱ
************************************************************************************************/
u32 WaitGPIOReset(uint32_t Portx, uint32_t Pin)
{
// 	if (GPIO_ReadInputDataBit(Portx, Pin) == RESET)
// 	{
// 		if (GET_PT_BITS(PT_TMP_NOTE_WM_ERR) == TRUE)
// 			SET_PT_BITS(PT_TMP_NOTE_WM_ERR, FALSE);
// 		return (TRUE);
// 	}else
// 	{
// 		if (GET_PT_BITS(PT_TMP_NOTE_WM_ERR) == FALSE)
// 			SET_PT_BITS(PT_TMP_NOTE_WM_ERR, TRUE);
// 		return (FALSE);
// 	}
    uint32_t i;
    for (i = 0; i < 65535; i++)
    {
        if (gpio_input_bit_get(Portx, Pin) == RESET)
        {
            return (TRUE);
        }
    }
    return (FALSE);
}
/***********************************************************************************************
** �� �� ����	RfidRxRegInit
** ����������	����CC1101�Ĵ���ΪRXģʽ
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void RfidRxRegInit(void)
{
    u8 u8FifoThr = s_u8RFAttenuationCfgTab[st_RFChipPar.u8RFAtnIdx] + s_u8RFCReg_FIFOTHR_BIT6[g_u8RFVelocityIdx];
    RfidWriteReg(CC1101_FIFOTHR, u8FifoThr);
    //RfidWriteReg(CC1101_FIFOTHR, s_aucRFAttenuationCfgTab[s_tRFChipPar.ucRFAtnIdx]);
    RfidWriteReg(CC1101_AGCCTRL2, 0xC7);
    RfidWriteReg(CC1101_AGCCTRL1, 0x00);
    RfidWriteReg(CC1101_MCSM0, 0x18);
    RfidWriteReg(CC1101_WORCTRL, 0xF8);
    RfidWriteReg(CC1101_MCSM2, 0x07);
}
/***********************************************************************************************
** �� �� ����	RfidWORRegInit
** ����������	����CC1101�Ĵ���ΪRXģʽ��Ӧ����
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void RfidWORRegInit(void)
{
	u8 u8MCSM2;
	
#define WOR_1200_MS
    u8 u8FifoThr = s_u8RFAttenuationCfgTab[st_RFChipPar.u8RFAtnIdx] + s_u8RFCReg_FIFOTHR_BIT6[g_u8RFVelocityIdx];
    RfidWriteReg(CC1101_FIFOTHR, u8FifoThr);
    //RfidWriteReg(CC1101_FIFOTHR, s_aucRFAttenuationCfgTab[s_tRFChipPar.ucRFAtnIdx] );
    RfidWriteReg(CC1101_AGCCTRL2, 0x03);
    RfidWriteReg(CC1101_AGCCTRL1, 0x40);
    RfidWriteReg(CC1101_MCSM0, 0x30);
    //RfidWriteReg(CC1101_MCSM0, 0x10);
    RfidWriteReg(CC1101_WORCTRL, 0x18);
    u8MCSM2 = s_u8RFCReg_MCSM2[g_u8RFVelocityIdx];
    RfidWriteReg(CC1101_MCSM2, u8MCSM2);//0x16
#ifdef WOR_1200_MS
    RfidWriteReg(CC1101_WOREVT1, 0xA2);
    RfidWriteReg(CC1101_WOREVT0, 0x80);
#else
    RfidWriteReg(CC1101_WOREVT1, 0x87);
    RfidWriteReg(CC1101_WOREVT0, 0x6B);
#endif
}

/***********************************************************************************************
** �� �� ����	RfidConfig()
** ����������	CC1101��������
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void RfidConfig(/*const*/ Tag_RF_CONFIG* ptRfConfig, const u8 u8RfPaTable)
{
    u8 u8Idx = 0;
    /*const*/ u8 *pu8RfConfig = (/*const*/ u8 *)ptRfConfig;

    switch (st_RFChipPar.u8RFVelocity)
    {
      case 38:/*38.4k*/
      {
		g_u8RFVelocityIdx = 1;
        break;
      }
      default:
        break;     
    }

    if (g_u8RFVelocityIdx) /*��250k��Ҫ�޸ļĴ���*/
    {
        *(pu8RfConfig + 5) = s_u8RFCReg_MDMCFG4[g_u8RFVelocityIdx]; /*MDMCFG4*/
        *(pu8RfConfig + 6) = s_u8RFCReg_MDMCFG3[g_u8RFVelocityIdx]; /*MDMCFG3*/
        *(pu8RfConfig + 11) = s_u8RFCReg_DEVIATN[g_u8RFVelocityIdx]; /*DEVIATN*/
        *(pu8RfConfig + 20) = s_u8RFCReg_FSCAL3[g_u8RFVelocityIdx];/*FSCAL3*/
        *(pu8RfConfig + 25) = s_u8RFCReg_TEST2[g_u8RFVelocityIdx];/*TEST2*/
        *(pu8RfConfig + 26) = s_u8RFCReg_TEST1[g_u8RFVelocityIdx];/*TEST1*/
    }
    
    for(u8Idx = 0; u8Idx < 34; u8Idx++)
    {
        RfidWriteReg(st_RfConfigIndex[u8Idx],  pu8RfConfig[u8Idx]);
    }
    RfidWriteReg(CC1101_PATABLE | CC1101_WRITE_BURST, u8RfPaTable);
}
/***********************************************************************************************
** �� �� ����	RFID_SPI_SendByte
** ����������	
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RFID_SPI_SendByte(u8 u8Byte)
{
	u32 i;
	
	i = 0;
	while ((spi_i2s_flag_get(RFID_SPI_PORT, SPI_FLAG_TBE) == RESET) && (i++<65536));

	spi_i2s_data_transmit(RFID_SPI_PORT, u8Byte);

	i = 0;
	while ((spi_i2s_flag_get(RFID_SPI_PORT, SPI_FLAG_RBNE) == RESET) && (i++<65536));

	return spi_i2s_data_receive(RFID_SPI_PORT);
}

/***********************************************************************************************
** �� �� ����	DrvRfChipReset
** ����������	
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void RfidChipReset(void)
{
    SpiResetChipSingle();
	
    RfidStrobe(CC1101_SRES);
}
/***********************************************************************************************
** �� �� ����	DrvSpiResetChipSingle
** ����������	
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 SpiResetChipSingle(void)
{
    /* оƬƬѡ�ź� */
	RFID_MCU_SCK_SET;
	RFID_MCU_SI_RESET;
    RFID_MCU_CS_DEASSERT;
    DelayUs(30);
    RFID_MCU_CS_ASSERT;
    DelayUs(100);
    RFID_MCU_CS_DEASSERT;
    DelayUs(45);
    return RET_OK;
}
/***********************************************************************************************
** �� �� ����	DrvSpiResetChipSingle
** ����������	
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
static u8 SpiMcuRead(u8 u8Addr, u8* pu8Data, u16 u16Length)
{
	u16 u16Index;
    u8 u8Status;
    
    /*
     * SPI׼������
     */
	//RFID_SPI_MCU_BEGIN;
	RFID_MCU_CS_ASSERT;
	DelayUs(150);		//DelayUs(300);
	u8Status = RFID_SPI_MCU_WAIT_MISO;
	if (u8Status == FALSE)
		return(TIME_OVER);
	u8Status = RFID_SPI_SendByte(u8Addr);
	DelayUs(1);
    /*
     *  ������
     */
    for (u16Index = 0; u16Index < u16Length; u16Index++)
    {
        pu8Data[u16Index] = RFID_SPI_SendByte(0);  /* Dummy write to read data byte */
		DelayUs(1);
    }
    /*
     *  SPI�����ݽ���
     */
    RFID_SPI_MCU_END;
	DelayUs(1);
    /*
     *  ���ض�д�Ƿ�ɹ�״̬
     */
    return(u8Status);
}
/***********************************************************************************************
** �� �� ����	RfidReadStatusReg
** ����������	
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ע�����	��ͨ��SPI��ȡ�ӿڶ�ȡһ��״̬�Ĵ�����ʱ��ͬʱ��ƵӲ��Ҳ�ڸ��¼Ĵ������ͻ���һ����С
**				�ģ����޵Ŀ��ܵ��½������ȷ��CC1100��CC2500��������ж�˵������һ���⣬���Ƽ��˼���
**				���ʵĹ�����
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RfidReadStatusReg(u8 u8Addr)
{
    u8 u8Reg;
	
    SpiMcuRead(u8Addr | CC1101_READ_BURST, &u8Reg, 1);
	
    return(u8Reg);
}
/***********************************************************************************************
** �� �� ����	RfidSpiWrite
** ����������	��CC1101д������
** �䡡  �룺	u8 u8Addr��д����׵�ַ
**				const u8* pu8Data��д�����ݵĵ�ַ
**				u16 u16Length��д�����ݳ���	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RfidSpiWrite(u8 u8Addr, const u8* pu8Data, u16 u16Length)
{
	u8 u8RetSta;
	
    u8RetSta = SpiMcuWrite(u8Addr, pu8Data, u16Length);

    return (u8RetSta);
}
/***********************************************************************************************
** �� �� ����	SpiMcuWrite
** ����������	��CC1101д������
** �䡡  �룺	u8 u8Addr��д����׵�ַ
**				const u8* pu8Data��д�����ݵĵ�ַ
**				u16 u16Length��д�����ݳ���
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
static u8 SpiMcuWrite(u8 u8Addr, const u8* pu8Data, u16 u16Length)
{
	u16 u16Index;
	u8 u8Status;
	//RFID_SPI_MCU_BEGIN;
	RFID_MCU_CS_ASSERT;
	DelayUs(150);		//DelayUs(300);
	u8Status = RFID_SPI_MCU_WAIT_MISO;
	if (u8Status == FALSE)
		return(TIME_OVER);
	u8Status = RFID_SPI_SendByte(u8Addr);
	DelayUs(1);
	for (u16Index = 0; u16Index < u16Length; u16Index++)
	{
		RFID_SPI_SendByte(pu8Data[u16Index]);
		DelayUs(1);
	}
	RFID_SPI_MCU_END;
	DelayUs(1);
	
	return(u8Status);
}
/***********************************************************************************************
** �� �� ����	RfidStrobe
** ����������	��CC1101���� strobe commands,����״̬
** �䡡  �룺	u8StrobeCmd��Ҫ���͵�����
** �䡡  ����	u8Status, ����״̬��Ϣ
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RfidStrobe(u8 u8StrobeCmd)
{
	u8 u8Status;
	#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0;
	#endif

	OS_ENTER_CRITICAL();//��ȫ���ж�
	if (CC1101_STX == u8StrobeCmd)
		vu32CC1100State = 1;
	else if(CC1101_SRX == u8StrobeCmd)
		vu32CC1100State = 2;
	else
		vu32CC1100State = 0;
    //RFID_SPI_MCU_BEGIN;
	RFID_MCU_CS_ASSERT;
	DelayUs(150);		//DelayUs(300);
	u8Status = RFID_SPI_MCU_WAIT_MISO;
	if (u8Status == FALSE)
	{
		OS_EXIT_CRITICAL();//��ȫ���ж�
		return(TIME_OVER);
	}
	u8Status = RFID_SPI_SendByte(u8StrobeCmd);
	DelayUs(1);
	RFID_SPI_MCU_END;
	//RFID_MCU_CS_DEASSERT;
	DelayUs(1);
	OS_EXIT_CRITICAL();//��ȫ���ж�
	
    return(u8Status);
}
/***********************************************************************************************
** �� �� ����	RFID_SpiPortInit
** ����������	
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void RfidReadAllStatusReg(u8* pu8Data, u16 u16Length)
{
	u8 u8i;
	
	for(u8i = 0; u8i < u16Length; u8i++)
	{
		pu8Data[u8i] = RfidReadStatusReg(StatusRegBaseAddress + u8i);
		DelayUs(1);
	}
}
/***********************************************************************************************
** �� �� ����	RfidWriteReg
** ����������	��CC1101оƬд�Ĵ�����ֵ
** �䡡  �룺	u8 u8Addr��д��Ĵ�����ַ��
** 				u8 u8Data��д�����ݣ�
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RfidWriteReg(u8 u8Addr, u8 u8Data)
{
    return(RfidSpiWrite(u8Addr, &u8Data, 1));
}
/***********************************************************************************************
** �� �� ����	RfidWriteFifo
** ����������	��TX-FIFOд��ָ�����ȵ�����
** �䡡  �룺	const u8 *pu8Data��д������Ҫ����ĵ�ַ��
** 				u8 u8Length��д�����ݳ��ȣ�
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RfidWriteFifo(const u8 *pu8Data, u8 u8Length)
{
    return(SpiMcuWrite(CC1101_TXFIFO | CC1101_WRITE_BURST, pu8Data, u8Length));
}
/***********************************************************************************************
** �� �� ����	RfidReadFifo
** ����������	��RX-FIFO��ȡָ�����ȵ�����
** �䡡  �룺	u8 *pu8Data����ȡ����Ҫ����ĵ�ַ��
** 				u8 u8Length����ȡ�����ݳ��ȣ�
** �䡡  ����	����״̬��Ϣ
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RfidReadFifo(u8 *pu8Data, u8 u8Length)
{
    return(SpiMcuRead(CC1101_RXFIFO | CC1101_READ_BURST, pu8Data, u8Length));
}
/***********************************************************************************************
** �� �� ����	RFCtrlSetIDLE
** ����������	
** �䡡  ����	����״̬��Ϣ
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RFCtrlSetIDLE(void)
{
    u16 u16TimeoutCnt = 0;
	
    u8 u8ChipStatus;
	
    /* 
	 * ��֤���ͽ�������֮ǰ�Ǵ���IDLE״̬
	 */
    RfidStrobe(CC1101_SIDLE);
    do
    {
        u8ChipStatus = RfidReadStatusReg(CC1101_MARCSTATE);
        u16TimeoutCnt ++;        
        DelayUs(10);
        if(u16TimeoutCnt > MAX_STATUS_DELAY)
        {
            return(CHIP_STATUS_ABNORMAL);
        }
    }while(u8ChipStatus != CC1101_MARCSTATE_IDLE);
    return RET_OK;
}

/***********************************************************************************************
** �� �� ����	SetRFChipIDLE
** ����������	��λ��ֱ��������ƵоƬ״̬ΪIDLE
** �䡡  �룺	u8 u8ProcPreSet��Ԥ����״̬��
** 				u8 u8ClearUpMode��Ҫ�����״̬��
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 SetRFChipIdle(u8 u8ProcPreSet, u8 u8ClearUpMode)
{
    u8 u8RetVal = 0;
	
    /*
	 * ����ǰ��λRFChip�����SIDLE
	 */
    if (RFCHIP_RESET_NECESSARILY == u8ProcPreSet)
    {
        RFID_Init();
		DelayUs(10);
        RfidStrobe(CC1101_SIDLE);/* �˳�RX/TX���ص�Ƶ�ʷ����� */
		DelayUs(10);
		RfidStrobe(CC1101_SCAL);
		DelayUs(10);
    }
    else
    {
        u8RetVal = RFCtrlSetIDLE();
        if(RET_OK != u8RetVal)
        {
             return CHIP_STATUS_ABNORMAL;
        }
    }
	
    if (RFCHIP_TX_CLEARUP == u8ClearUpMode)
    {
        RfidStrobe(CC1101_SFTX);    // ˢ�� TX FIFO buffer.
		//DelayUs(10);
    }
    else
    {
        RfidStrobe(CC1101_SFRX);    // ˢ�� RX FIFO buffer.
		//DelayUs(10);
    }
    
    return RET_OK;
}
/***********************************************************************************************
** �� �� ����	RFTxSendPacket
** ����������	��CC1101оƬд���ݰ�
** �䡡  �룺	u8* pu8Data��д�����ݵ�ַ��
** 				u8 u8Length��д�����ݳ��ȣ�
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RFTxSendPacket(u8* pu8Data, u8 u8Length)
{	
//	u32 u32i;
	//static u8 u8LastLen=0;
	#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0;
	#endif

    if (u8Length >= CC1101_TX_FIFO_SIZE)
    {
        return (TX_LENGTH_ERR);
    }
    
	OS_ENTER_CRITICAL();//��ȫ���ж�
    /* ��֤���ͽ�������֮ǰ�Ǵ���IDLE״̬*/
    if(RET_OK != SetRFChipIdle(RFCHIP_IDLE_POSITIVE, RFCHIP_TX_CLEARUP))
    {
		OS_EXIT_CRITICAL();//��ȫ���ж�
        return CHIP_STATUS_ABNORMAL;
    }
	//DelayUs(10);

	RfidStrobe(CC1101_SFTX);
	DelayUs(10);

	vu32GDO0IntFlag = 0;

	//RfidWriteReg(CC1101_PKTLEN,u8Length);
	DelayUs(10);
	
	// ��TX FIFO��д�����ݣ�ֻ���ڿ���״̬�ſ���
    RfidWriteFifo(pu8Data, u8Length+1);
	//DelayUs(10);
	OS_EXIT_CRITICAL();//��ȫ���ж�

	// ����CC1101�ķ���״̬
    RfidStrobe(CC1101_STX);

	
	// �ж�һ�������Ƿ������
// 	u32i = 0;
// 	while(u32i++<0x20000)
// 	{
// 		if(vu32GDO0IntFlag)
// 		{
// 			vu32GDO0IntFlag = 0;
// 			DelayUs(10);
// 			break;
// 		}
// 	}
// //	RfidStrobe(CC1101_SFTX);
// 	if (u32i<0x20000)
		return TX_OK;
// 	return(TIME_OVER);
}
/***********************************************************************************************
** �� �� ����	RfidReadFifoProc
** ����������	��CC1101 FIFO����ȡһ�����������ݰ�
** �䡡  �룺	u8 *pu8Data�����뱣��������ݵĵ�ַ��
** 				u8 *pu8Length�����뱣��������ݳ��ȵĵ�ַ��
** �䡡  ����	return 	RX_CRC_MISMATCH����ʾCRCУ�鲻��ȷ��
**						RX_OK,��ʾ�������ݳɹ�
**						ע�⣬���յ����ݻᱣ������Ӧ�Ļ��棬�Լ����ݳ��ȣ�*pu8Length = �������� + 1(*pu8Length����ռ��һ���ֽ�)
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RfidReadFifoProc(u8 *pu8Data, u8 *pu8Length)
{
#if 1
//    u8 u8CurrentRxBytes, u8LastRxBytes,u8DataLen,u8LastDataLen;
    u8 u8DataLen;
    u8 u8AppendStatus[2];
//    u16 u16TimeoutCnt = 0;
    
	/* 
     * ��ȡfifo�ֽڳ��ȣ�CC1101_RXBYTES��״̬�Ĵ�����һ��
     */
// 	u8CurrentRxBytes = RfidReadStatusReg(CC1101_RXBYTES);
//     do
//     {
//         u8LastRxBytes = u8CurrentRxBytes; 
//         u8CurrentRxBytes = RfidReadStatusReg(CC1101_RXBYTES);		
//         u16TimeoutCnt ++;
//         DelayUs(100);
//         if(u16TimeoutCnt > MAX_STATUS_DELAY)
//         {
//             return(CHIP_STATUS_ABNORMAL);
//         }  
//     }while((u8CurrentRxBytes < CC1101_RX_FIFO_SIZE) && (u8CurrentRxBytes != u8LastRxBytes));
//     
// 	// �������������������ս��ջ���
// 	if(u8CurrentRxBytes & 0x80)
// 	{
// 		RfidStrobe(CC1101_SFRX);
// 		return(RX_LENGTH_VIOLATION);
// 	}
	DelayUs(1);
	//��ȡ����
	RfidReadFifo(&u8DataLen, 1);				// ��ȡ���ݰ������ֽ�
	DelayUs(1);
	//if(u8DataLen && (u8LastDataLen != u8DataLen))
	//{
	//	RfidWriteReg(CC1101_PKTLEN,u8DataLen);
	//	u8LastDataLen = u8DataLen;
	//	DelayUs(100);
	//}
	RfidReadFifo(pu8Data, u8DataLen);			// ��ȡ��������	
	DelayUs(1);
	//*pu8Length = u8DataLen;					// �������ݳ���	
	RfidReadFifo(u8AppendStatus, 2);			// CRCУ��
	DelayUs(1);
	
	pu8Data[u8DataLen] = u8AppendStatus[0];
	pu8Data[u8DataLen + 1] = u8AppendStatus[1];
	
	*pu8Length = u8DataLen + 2;					// �������ݳ���
	// ���������ݣ�Ҳ������ս��ջ��棬ֻ����SIDLE��SRX�����ʱ������
	//DelayUs(50);
	RfidStrobe(CC1101_SIDLE);
	DelayUs(1);
	RfidStrobe(CC1101_SFRX);
	DelayUs(1);
	SetRxMode();
	
	// �ж�CRCУ���Ƿ���ȷ������ȷ����RX_CRC_MISMATCH;���򷵻�RX_OK;
    if ((u8AppendStatus[1] & CC1101_LQI_CRC_OK_BM) != CC1101_LQI_CRC_OK_BM)
    {
        return(RX_CRC_MISMATCH);
    }
    return(RX_OK);
#endif // 0
}
/***********************************************************************************************
** �� �� ����	RfidGetTxStatus
** ����������	This function transmits a No Operation Strobe (SNOP) to get the status of
**				the radio and the number of free bytes in the TX FIFO
**				״̬�ֽڣ�
**      		---------------------------------------------------------------------------
**      		|          |            |                                                 |
**      		| CHIP_RDY | STATE[2:0] | FIFO_BYTES_AVAILABLE (free bytes in the TX FIFO |
**      		|          |            |                                                 |
**      		---------------------------------------------------------------------------				
** �䡡  �룺	
** �䡡  ����	
** ע�����	������Ӳ�����ڸ��¼Ĵ�����ʱ��ͬʱ����ͨ��SPI�ӿڶ�ȡ״̬�Ĵ�����������һ���ǳ�С�ģ�
**				���޵Ŀ��ܵ��½���ǲ���ȷ�ġ�ͬ�������Ҳ����оƬ״̬�ֽڡ�CC1100��CC2500���������
**				������������⣬���ṩ�˼��ֹ�����
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u8 RfidGetTxStatus(void)
{
    return(RfidStrobe(CC1101_SNOP));
}

//---------------------------------------------------------------------------------------------
//---			CC1101������ز���
//---------------------------------------------------------------------------------------------
/***********************************************************************************************
** �� �� ����	SetRFChipSleep
** ����������	������ƵоƬ����
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void SetRFChipSleep(void)
{
	
    RfidRxRegInit();
	//DelayUs(10);
    /*
     * ��CSn����ʱ������˯��ģʽ�����رյ�Դģʽ
     */
    RfidStrobe(CC1101_SPWD);
	//DelayUs(10);
}
/***********************************************************************************************
** �� �� ����	SetRFChipWOR
** ����������	������ƵоƬWOR
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void SetRFChipWOR(void)
{
    RfidWORRegInit();
	
    /*
	 * ������ڲ��������붨ʱ���ѽ���״̬
	 */
    RfidStrobe(CC1101_SWORRST);    			// Reset real time clock.
	//DelayUs(10);
    RfidStrobe(CC1101_SWOR);    			// Start automatic RX polling sequence (Wake-on-Radio).
	//DelayUs(10);
}
/***********************************************************************************************
** �� �� ����	SetRFChipSleepMode
** ����������	������ƵоƬ˯��ģʽ
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void SetRFChipSleepMode(void)
{
//    u16 u16CCR;
	
    /*����ʵ���������CC1101����״̬*/
    if(g_u16RunningMode == OBUMODE_SLEEP)
    {
        SetRFChipSleep();
		//DelayUs(10);
//		if (st_RFChipPar.u8ResIntervalTime < 3)
//		{
//			u16CCR = 125;				/*100ms*//* WOR_TIMER_CLK */
//		}
//		else
//		{
//			 /*Ŀǰ���52s��������������ACLK��Ƶ����*/
//			u16CCR = st_RFChipPar.u8ResIntervalTime * 1250;/* WOR_TIMER_CLK */
//		}
		//WORTimerStart(u16CCR);
    }
    else
    {
        SetRFChipWOR();
		//DelayUs(10);
		//g_wRunningMode = OBUMODE_SLEEP;
    }
}
#if 0
/**************************************************************************
* ��������: void HFWakeupProc(void)
* ��������: ��Ƶ�����жϴ�������
* ���ʵı�: ��
* �޸ĵı�: ��
* �������: 
* �������: ��
* �� �� ֵ: 
* ����˵��: ��
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
**************************************************************************/
void HFWakeupProc(void)
{
    //drv_IoToggle(&s_tPinLed);
    RfCtrl_ClearHFInt();
    MsgQSendFromIsr(HighPriority, RFCommRcvTaskID, APP_COMMRCV_HFWAKEUP_MSG_ID, NULL);
    DRV_LPM_EXIT;
}
/**************************************************************************
* ��������: void RFPacketRcvdProc(void)
* ��������: ����Ƶ�����жϴ�������
* ���ʵı�: ��
* �޸ĵı�: ��
* �������: 
* �������: ��
* �� �� ֵ: 
* ����˵��: ��
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
**************************************************************************/
void RFPacketRcvdProc(void)
{
    //drv_IoToggle(&s_tPinLed);
    RfCtrl_ClearRXInt();
    MsgQSendFromIsr(HighPriority, RFCommRcvTaskID, APP_COMMRCV_RFRCVOK_MSG_ID, NULL);
    DRV_LPM_EXIT;
}
/**************************************************************************
* ��������: BYTE RFTimeLimitRecv(void)
* ��������: ��ʱ���ѽ��պ���9ms
* ���ʵı�: ��
* �޸ĵı�: ��
* �������: 
* �������: ��
* �� �� ֵ: 
* ����˵��: ��
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
**************************************************************************/
BYTE RFTimeLimitRecv(void)
{  
    WORD16 wTrVal = 0;
    /*����дCC1101�Ĵ���*/
    if(USP_SUCCESS != SetRFChipIDLE(RFCHIP_RESET_NECESSARILY, RFCHIP_RX_CLEARUP))
    {
        return CHIP_STATUS_ABNORMAL;
    }
    InitRFReg_RX();
    g_wHFWakeUpTimes++;
    /*������Ƶ���ѣ�������������״̬*/
    RfCtrl_ClearRXInt();
    drvRfStrobe(CC1101_SRX); 
    /*������ʱ���ն�ʱ��*/
    drv_TimerStart(&s_tRFRcvTimerCfg, NULL);
    while(1)
    {
        if(IFRXIFG)
        {               
            RfCtrl_ClearRXInt();
            drvRfStrobe(CC1101_SIDLE);
            drv_TimerStop(&s_tRFRcvTimerCfg);
            drv_TimerClear(&s_tRFRcvTimerCfg);
            return RX_NORMAL_OK;
        }
        
        drv_TimerGetCount(&s_tRFRcvTimerCfg, &wTrVal);
        if(wTrVal >= RFRCV_TIMER_COUNT)
        {
             g_wErrorWakeUpTimes++;
             drvRfStrobe(CC1101_SIDLE);
             drv_TimerStop(&s_tRFRcvTimerCfg);
             drv_TimerClear(&s_tRFRcvTimerCfg);
             return RX_NORMAL_TIMEOUT;
        }
    }
}
//---------------------------------------------------------------------------------------------
//---			RFID �������
//---------------------------------------------------------------------------------------------
/**************************************************************************
* ��������: void RFCommRcvTask(BYTE ucMsgId, void *pMsgptr)
* ��������: ��Ƶ��������
* ���ʵı�: ��
* �޸ĵı�: ��
* �������: BYTE ucMsgId,   ��ϢID��
*           void *pMsgptr,  ָ����Ϣ��ָ�룻
* �������: ��
* �� �� ֵ: 
* ����˵��: ��
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
**************************************************************************/
void RFCommRcvTask(BYTE ucMsgId, void *pMsgptr)
{
    BYTE ucRFRcvOK = FALSE;
    //WORD16 wCrcCalc = 0;
    //WORD16 wCrcRcv = 0;
    BYTE *pucMsgLen = NULL;
    BYTE *pucMsgData = NULL;

    tNumInfo.dwRfidWakeupNum++;
    drv_IoSet(&s_tRxLed);
    memset(g_aucRFCommBuf, 0, RF_COMM_BUF_LEN);
	
    if (APP_COMMRCV_RFRCVOK_MSG_ID == ucMsgId)
    {
        pucMsgLen = &g_aucRFCommBuf[0];
        pucMsgData = &g_aucRFCommBuf[1];
        ucRFRcvOK = RFReadFifoProc(pucMsgData, pucMsgLen);
        //if(!(tNumInfo.dwRfidWakeupNum%5))
           //ucRFRcvOK = RX_LENGTH_VIOLATION;
        if(RX_OK != ucRFRcvOK)
        {   
            if(CHIP_STATUS_ABNORMAL == ucRFRcvOK)
            {
                 tNumInfo.dwChipAbnormal++;
                RFChipAbnormalProc(RFCHIP_RX_CLEARUP);
            }
	    else if (RX_LENGTH_VIOLATION == ucRFRcvOK)
		tNumInfo.dwLengthErr++;
	    else if (RX_CRC_MISMATCH == ucRFRcvOK)
        	tNumInfo.dwCrcErr++;
	    drv_IoClear(&s_tRxLed);
            return;
        }
#if 0
        //У��CRC
        wCrcCalc = CommVerify((*pucMsgLen)-2, pucMsgData);
        wCrcRcv = ((WORD16)(pucMsgData[(*pucMsgLen)-2]) << 8) | ((WORD16)(pucMsgData[(*pucMsgLen-1)]));
        if(wCrcCalc != wCrcRcv)
        {
            return;
        }
#endif
	tNumInfo.dwRfidRcvNum++;
        
        /*
         * ������������
         */
        MsgQSend(HighPriority, MainProcessTaskID, APP_MAINPROCESS_PROC_MSG_ID, g_aucRFCommBuf);
    }
}
/**************************************************************************
* ��������: WORD16 CommVerify(BYTE ucLength, BYTE *pucCheckSource)
* ��������: ����У��
* ���ʵı�: ��
* �޸ĵı�: ��
* �������: 
* �������: ��
* �� �� ֵ: 
* ����˵��: ��
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
**************************************************************************/
WORD16 CommVerify(BYTE ucLength, BYTE *pucCheckSource)
{
    WORD16 wVerifyNum = CRC_PRESET ;            // CRC_PRESET= 0xffff
    BYTE ucVerifyi,ucVerifyj ;
    for (ucVerifyi=0;ucVerifyi<ucLength;ucVerifyi++)
    {
      wVerifyNum^=pucCheckSource[ucVerifyi] ;
      for (ucVerifyj=0;ucVerifyj<8;ucVerifyj++)
      {
        if (wVerifyNum&0x0001)
          wVerifyNum=(wVerifyNum>>1)^CRC_POLYNOM ; // CRC_POLYNOM=8408H
        else
          wVerifyNum=(wVerifyNum>>1) ;
      }
    }
    return wVerifyNum  ;
}
#endif // 0
/***********************************************************************************************
** �� �� ����	ClrRxFifo
** ����������	������ջ���
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void ClrRxFifo(void)
{	
    RfidStrobe(CC1101_SFRX);		//�������״̬
	DelayUs(1);
}
/***********************************************************************************************
** �� �� ����	SetRxMode
** ����������	EXTI INTERRUPT �����Ӻ���
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void SetRxMode(void)
{	
// 	RfidRxRegInit();
// 	DelayUs(1);
// 	//RfidWriteReg(CC1101_PKTLEN,0x40);			//���ý��ճ���
// 	//DelayUs(1);
//     RfidStrobe(CC1101_SRX);		//�������״̬
// 	DelayUs(1);
	u32	i;
	#if OS_CRITICAL_METHOD == 3                 /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0;
	#endif

	OS_ENTER_CRITICAL();//��ȫ���ж�
	RfidRxRegInit();
	for (i = 0;i < 20;)
		i++;
	//RfidWriteReg(CC1101_PKTLEN,0x40);		//���ý��ճ���
	//for (i = 0;i < 20;)
	//	i++;
    RfidStrobe(CC1101_SRX);		//�������״̬	
	for (i = 0;i < 20;)
		i++;
	OS_EXIT_CRITICAL();//��ȫ���ж�
}

/***********************************************************************************************
** �� �� ����	RFID_GetFrame
** ����������	�ӽ��ջ�����ȡ��һ�����ݰ�
** �䡡  �룺	
** �䡡  ����	
** ����  �ߣ�	����
** �ա�  �ڣ�	2014.12.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 RFID_GetFrame(st_RFIDRcvFrame *RfidRcvFrm)
{
	if(u32RcvFrmPoolCnt)
	{
		#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
		OS_CPU_SR  cpu_sr = 0;
		#endif

		OS_ENTER_CRITICAL();//��ȫ���ж�
		*RfidRcvFrm = st_RFIDRcvFrmPool[u32RcvFrmPoolReadPtr++];
		u32RcvFrmPoolReadPtr %= RFID_RCV_FRM_SIZE;
		
		u32RcvFrmPoolCnt--;
		OS_EXIT_CRITICAL();//��ȫ���ж�
		return RET_OK;
	}else
	{
		return RET_ERR;
	}
}
//---------------------------------------------------------------------------------------------
//---			EXTI �����Ӻ���
//---------------------------------------------------------------------------------------------
/***********************************************************************************************
** �� �� ����	EXTI0_IRQHandler
** ����������	EXTI INTERRUPT �����Ӻ���
** �䡡  �룺
** �䡡  ����
************************************************************************************************/
void EXTI3_IRQHandler(void)
{
//	u32 u32i;
//	st_RFIDRcvFrame RfidRcvFrmTmp;
	FlagStatus GDO0EXTIStatus;
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif

	OS_ENTER_CRITICAL();
	OSIntEnter();
	OS_EXIT_CRITICAL();
	
	/* Get the status of RFID_GDO0_EXTI_LINE */
	GDO0EXTIStatus = exti_flag_get(RFID_GDO0_EXTI_LINE);
	if (GDO0EXTIStatus)
	{
		if (vu32CC1100State == 1)
		{
			vu32GDO0IntFlag = 1;
			vu32CC1100State = 2;			//��������Զ�ת�����״̬���Ĵ������ã�
		}
 		else if (vu32CC1100State == 2)
 		{
 			u8 u8CurrentRxBytes;
 			u8CurrentRxBytes = RfidReadStatusReg(CC1101_RXBYTES);
 			// �������������������ս��ջ���
 			if(u8CurrentRxBytes & 0x80)
 			{
 				RfidStrobe(CC1101_SFRX);	//������ջ�����
 				RfidStrobe(CC1101_SRX);		//�������״̬	
 			}
 		}
			
		/* Clear the RFID_GDO0_EXTI_LINE pending flag */
		exti_flag_clear(RFID_GDO0_EXTI_LINE);
	}
	OSIntExit();
}
/***********************************************************************************************
** �� �� ����	EXTI3_IRQHandler
** ����������	EXTI INTERRUPT �����Ӻ���
** �䡡  �룺
** �䡡  ����
************************************************************************************************/
void EXTI2_IRQHandler(void)
{
	u32 u32i;
	st_RFIDRcvFrame RfidRcvFrmTmp;
	FlagStatus GDO2EXTIStatus;
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif

	OS_ENTER_CRITICAL();
	OSIntEnter();
	OS_EXIT_CRITICAL();
	
	/* Get the status of RFID_GDO2_EXTI_LINE */
	GDO2EXTIStatus = exti_flag_get(RFID_GDO2_EXTI_LINE);
	if (GDO2EXTIStatus)
	{
		/* Clear the RFID_GDO2_EXTI_LINE pending flag */
		exti_flag_clear(RFID_GDO2_EXTI_LINE);

		//��ȫ���жϣ�
		//OS_ENTER_CRITICAL();
		u32i = RfidReadFifoProc(u8RcvData, &u8RcvLength);
		//��ȫ���жϣ�
		//OS_EXIT_CRITICAL();
			
		if (u32i == RX_OK)
		{
#if (CC1101_ADDR_FILTER > 0)
			if (u8RcvLength > 3)
			{
				RfidRcvFrmTmp.u8DataLen = u8RcvLength - 3;		// u8RcvLength = 1��ַ�ֽ� + N�����ֽ� + 2��״̬�ֽ�
				RfidRcvFrmTmp.u8DestAddr = u8RcvData[0];
				for(u32i = 0;u32i < RfidRcvFrmTmp.u8DataLen && u32i < 64;u32i++)
				{
					RfidRcvFrmTmp.u8Data[u32i] = u8RcvData[u32i + 1];
				}
#else
			if (u8RcvLength > 2)
			{
				RfidRcvFrmTmp.u8DataLen = u8RcvLength - 2;		// u8RcvLength = N�����ֽ� + 2��״̬�ֽ�
				RfidRcvFrmTmp.u8DestAddr = 0x00;
				for(u32i = 0;u32i < RfidRcvFrmTmp.u8DataLen && u32i < 64;u32i++)
				{
					RfidRcvFrmTmp.u8Data[u32i] = u8RcvData[u32i];
				}
#endif	
				RfidRcvFrmTmp.u8AppendStatus[0] = u8RcvData[u8RcvLength-2];
				RfidRcvFrmTmp.u8AppendStatus[1] = u8RcvData[u8RcvLength-1];
			
				if (u32RcvFrmPoolCnt < RFID_RCV_FRM_SIZE)
				{
					// �����ݰ����뵽�������
					st_RFIDRcvFrmPool[u32RcvFrmPoolWritePtr++] = RfidRcvFrmTmp;
					u32RcvFrmPoolWritePtr %= RFID_RCV_FRM_SIZE;
					
					u32RcvFrmPoolCnt++;
					// ���յ����ݺ�Ļص�����
					if (RcvedBackCallFunc != NULL)
						(*RcvedBackCallFunc)();
				}
			}
		}
	}
	OSIntExit();
}

/*********************************������������޹�˾*************************************************************/