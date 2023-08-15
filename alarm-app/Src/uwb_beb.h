#ifndef __UWP_BEB_H__
#define __UWP_BEB_H__


#include "stdint.h"
#include "stdbool.h"
#include "can_app.h"
#include "deca_device_api.h"
#include "deca_regs.h"
//#include "filtering.h"

#define COLLISION_WINDOW_MAX   64
#define COLLISION_WINDOW_MIN   1

#define BACK_OFF_TIME_MS       1
#define LISTEN_CHANNEL_TIME		 200		//信道监听时间200us
#define BACK_OFF_COUNT_MAX     16


typedef enum
{
	CHANNEL_BUSY = 0x00,
	CHANNEL_FREE = 0x01
}ChannelStatus;

#endif /*__UWP_BEB_H__*/
