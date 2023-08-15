#ifndef __RANDOM_APP_H__
#define __RANDOM_APP_H__

#include "gd32f30x.h"
//#include "stm32f4xx_hal.h"
#if INVOKED		//声明实际未被调用
void RNG_Init(void);
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng);
uint32_t RNG_Get_RandomNum(void);
uint16_t RNG_Get_RandomRange(uint16_t min, uint16_t max);

#endif

#endif /*__RANDOM_APP_H__*/
