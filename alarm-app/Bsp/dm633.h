/***************************************************************************
* 版权所有：2014, 天津华宁电子有限公司
*
* 文件名称：dm633.h
* 文件标识：
* 内容摘要：LED Grayscale Driver IC
* 其它说明：
* 当前版本：

*
* 修改记录1：
*    修改日期：
*    版 本 号：
*    修 改 人：
*    修改内容：
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
#define GPIO_PORT_DM633_DEVICE1_LAT			GPIOC			      /* GPIO端口 */
#define DM633_DEVICE1_LAT_PIN				GPIO_PIN_2			  /* 连接到LAT的GPIO */

#define RCU_PORT_DM633_DEVICE2_LAT			RCU_GPIOC
#define GPIO_PORT_DM633_DEVICE2_LAT			GPIOC			      /* GPIO端口 */
#define DM633_DEVICE2_LAT_PIN				GPIO_PIN_7			  /* 连接到LAT的GPIO */

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
//LED电源控制
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

//单独控制其中一个点亮
void ctrlUled_RGBValue_D1(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
//控制两个连续的灯点亮
void ctrlUled_RGBValue_D1_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
//控制四个连续的灯点亮
void ctrlUled_RGBValue_D1_D3_D5_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D2_D4_D6_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
//控制连续的灯点亮
void ctrlUled_RGBValue_D1_D2_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D1_D2_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D1_D2_D3_D4_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
//控制连续的四个灯点亮，亮度值递减
void ctrlUled_RGBValue_Meteor_D1_D2_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D2_D3_D4_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D3_D4_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D4_D5_D6_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D5_D6_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D6_D7_D8_D1(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D7_D8_D1_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D8_D1_D2_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
//控制连续的三个灯点亮，亮度值递减
void ctrlUled_RGBValue_Meteor_D1_D2_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D2_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D3_D4_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D4_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D5_D6_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D6_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D7_D8_D1(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
void ctrlUled_RGBValue_Meteor_D8_D1_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale);
#endif
