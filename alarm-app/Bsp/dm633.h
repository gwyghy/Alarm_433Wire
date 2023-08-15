/***************************************************************************
* ��Ȩ���У�2014, ������������޹�˾
*
* �ļ����ƣ�dm633.h
* �ļ���ʶ��
* ����ժҪ��LED Grayscale Driver IC
* ����˵����
* ��ǰ�汾��

*
* �޸ļ�¼1��
*    �޸����ڣ�
*    �� �� �ţ�
*    �� �� �ˣ�
*    �޸����ݣ�
**********************************************************************/
#ifndef  __dm633__ 
#define  __dm633__ 


#include "string.h"
#include "stdlib.h"	
//#include "stm32f4xx.h"
//#include "stm32f4xx_hal.h"
#include "gd32f30x.h"


#define DEVICE_RED			1
#define DEVICE_BLUE			2

#define RCU_PORT_DM633_DEVICE1_LAT			RCU_GPIOC
#define GPIO_PORT_DM633_DEVICE1_LAT			GPIOC			      /* GPIO�˿� */
#define DM633_DEVICE1_LAT_PIN				GPIO_PIN_2			  /* ���ӵ�LAT��GPIO */

#define RCU_PORT_DM633_DEVICE2_LAT			RCU_GPIOC
#define GPIO_PORT_DM633_DEVICE2_LAT			GPIOC			      /* GPIO�˿� */
#define DM633_DEVICE2_LAT_PIN				GPIO_PIN_7			  /* ���ӵ�LAT��GPIO */

#define RCU_PORT_LED_POWER					RCU_GPIOB
#define GPIO_PORT_LED_POWER					GPIOB
#define LED_POWER_PIN						GPIO_PIN_9

#define MOS_ENABLED				1
#if MOS_ENABLED == 1
	#define I2C_LAT_1_1()  GPIO_BC(GPIO_PORT_DM633_DEVICE1_LAT) = (uint32_t)DM633_DEVICE1_LAT_PIN			/* LAT = 0 */
	#define I2C_LAT_1_0()  GPIO_BOP(GPIO_PORT_DM633_DEVICE1_LAT) = (uint32_t)DM633_DEVICE1_LAT_PIN			/* LAT = 1 */

	#define I2C_LAT_2_1()  GPIO_BC(GPIO_PORT_DM633_DEVICE2_LAT) = (uint32_t)DM633_DEVICE2_LAT_PIN			/* LAT2 = 0 */
	#define I2C_LAT_2_0()  GPIO_BOP(GPIO_PORT_DM633_DEVICE2_LAT) = (uint32_t)DM633_DEVICE2_LAT_PIN			/* LAT2 = 1 */
#else
	#define I2C_LAT_1_0()  GPIO_BC(GPIO_PORT_DM633_DEVICE1_LAT) = (uint32_t)DM633_DEVICE1_LAT_PIN			/* LAT = 0 */
	#define I2C_LAT_1_1()  GPIO_BOP(GPIO_PORT_DM633_DEVICE1_LAT) = (uint32_t)DM633_DEVICE1_LAT_PIN			/* LAT = 1 */

	#define I2C_LAT_2_0()  GPIO_BC(GPIO_PORT_DM633_DEVICE2_LAT) = (uint32_t)DM633_DEVICE2_LAT_PIN			/* LAT2 = 0 */
	#define I2C_LAT_2_1()  GPIO_BOP(GPIO_PORT_DM633_DEVICE2_LAT) = (uint32_t)DM633_DEVICE2_LAT_PIN			/* LAT2 = 1 */
#endif
//LED��Դ����
#define LED_POWER_ON()		GPIO_BOP(GPIO_PORT_LED_POWER) = (uint32_t)LED_POWER_PIN						/* POWER_ON = 1 */
#define LED_POWER_OFF()		GPIO_BC(GPIO_PORT_LED_POWER) = (uint32_t)LED_POWER_PIN						/* POWER_ON = 0 */

void DM633Init(void);
void TurnOffAllUledPro(uint8_t device);
void ctrlUledPro(uint8_t device, uint16_t grayscale);
void TurnOnUledPro(uint8_t device, uint16_t grayscale);

void ctrlUledTabValue_RED(uint8_t device);
void ctrlUledTabValue_BLUE(uint8_t device);
void ctrlUledTabValue_GREEN(uint8_t device);

void ctrlUledRGBValue(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);

void ClrBreatheSta(void);
void ctrlUledRGBValueBreathe(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale, uint16_t rstep, uint16_t gstep, uint16_t bstep);

//������������һ������
void ctrlUled_RGBValue_D1(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
//�������������ĵƵ���
void ctrlUled_RGBValue_D1_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
//�����ĸ������ĵƵ���
void ctrlUled_RGBValue_D1_D3_D5_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D2_D4_D6_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
//���������ĵƵ���
void ctrlUled_RGBValue_D1_D2_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D1_D2_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D1_D2_D3_D4_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
//�����������ĸ��Ƶ���������ֵ�ݼ�
void ctrlUled_RGBValue_Meteor_D1_D2_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D2_D3_D4_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D3_D4_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D4_D5_D6_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D5_D6_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D6_D7_D8_D1(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D7_D8_D1_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D8_D1_D2_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
//���������������Ƶ���������ֵ�ݼ�
void ctrlUled_RGBValue_Meteor_D1_D2_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D2_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D3_D4_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D4_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D5_D6_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D6_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D7_D8_D1(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D8_D1_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
#endif
