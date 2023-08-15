#ifndef __INCLUDES_H
#define __INCLUDES_H

#define	SC_RID								0
#define EMVD_RID							1
#define HUB_RID								2
#define WL_RID								3
#define CXB_RID								4
#define RID_MASK							0x00000007
#define RID_BITS							0x00000007
#define CAN1_FILTER_ID_HIGH					(u16)(((WL_RID << 3) >> 16) & 0xffff)	//过滤器中标识符ID
#define CAN1_FILTER_ID_LOW					(u16)(((WL_RID << 3) & 0xffff) | CAN_ID_EXT | CAN_RTR_DATA)
#define CAN1_FILTER_MASK_ID_HIGH			(u16)(((RID_BITS << 3) >> 16) & 0xffff)	//过滤器中屏蔽位掩码ID
#define CAN1_FILTER_MASK_ID_LOW				(u16)(((RID_BITS << 3) & 0xffff) | CAN_ID_EXT | CAN_RTR_DATA)


#include "rfid_cc1101.h"
#include "rfid_driver.h"
#include "rfid_config.h"

#include "wireless.h"

#include "ucos_ii.h"
#include "os_cfg.h"
#include "os_cpu.h"


#include "string.h"
#include "stdlib.h"	
#include "gd32f30x.h"
#endif
