#ifndef __UWB_DEBUG_H__
#define __UWB_DEBUG_H__


#include "stdint.h"
#include "stdbool.h"
#include "port.h"
#include "uCOS_II.h"

#define UWB_DEBUG_ENABLE                            0

#define DEBUG_TIME_DELAY                           2000


#define   UWB_DEBUG_STK_SIZE                       128


void CreatUwbDebugTask   (void);
void SetDebugCount(void);
#endif /*__UWB_DEBUG_H__*/
