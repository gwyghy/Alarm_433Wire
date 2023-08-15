#ifndef __BEEP_H__
#define __BEEP_H__


#include "string.h"
#include "stdlib.h"	
#include "gd32f30x.h"
//#include "stm32f4xx.h"
//#include "stm32f4xx_hal.h"
//∑‰√˘∆˜IOœ‡πÿ≈‰÷√
#define	BUZZER_GPIO					RCU_GPIOA
#define	BUZZER_PORT					GPIOA
#define BUZZER_PIN					GPIO_PIN_9

u8 GetCurrentBeepSta(void);
void ClrCurrentBeepSta(void);
void bsp_InitBeep(void);
void AppTaskBeep(void *p_arg);

#endif

