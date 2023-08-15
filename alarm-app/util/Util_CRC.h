/*
*********************************************************************************************************
*	                                            V4
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : Bsp_Time.h
*    Module  : user
*    Version : V1.0
*    History :
*   -----------------
*              Version  Date           By            Note
*              v1.0         
*
*********************************************************************************************************
*/

#ifndef __UTIL_CRC_H__
#define __UTIL_CRC_H__

#include "includes.h"

void Crc16Ccitt(uint8_t *u8Buf, uint32_t u32Len, uint16_t *u16CheckOld);
#endif


