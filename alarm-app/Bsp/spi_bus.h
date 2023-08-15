/*! ----------------------------------------------------------------------------
 * @file	spi_bus.h
 * @brief	stm32l452 spi Çý¶¯
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#ifndef __SPI_BUS_H__
#define __SPI_BUS_H__		

#include "l4spi.h"


void SpiBusDataInit(void);

int ReadFromSPI(uint16_t headerLength,  uint8_t *headerBuffer, uint32_t readlength, uint8_t *readBuffer);
int WriteToSPI(uint16_t headerLength, uint8_t *headerBuffer, uint32_t bodylength, uint8_t *bodyBuffer);

void SpiBusInit(void);

#endif  /*__SPI_BUS_H__*/
	 



