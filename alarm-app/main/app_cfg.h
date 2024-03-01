/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                           STM3240G-EVAL
*                                         Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : FT
*                 DC
*********************************************************************************************************
*/

#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/

//软件版本信息上报
#define VERSION_1    4
#define VERSION_2    0
#define VERSION_3    8

/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/
/*** 0、1、2、3预留给uc/os系统***************/
#define CANRX_TASK_PRIO							4u      /* can receive  task   */
#define CANTX_TASK_PRIO							10u     /* can transmit task   */
//#define UWBRX_TASK_PRIO						5u	    /*uwb接收任务优先级 */
//#define  UWB_DEBUG_TASK_PRIO					5u	    /*uwb调试任务优先级 */
#define UWB_ANCHOR_TASK_PRIO					5u
//#define UWB_TAG_TASK_PRIO						7u
//#define  TAG_REPORT_TAST_PRIO					8u
#define	 UWBAPP_TASK_PRIO						6u

#define APP_TASK_LIGHT_PRIO						9u

#define APP_TASK_BEEP_PRIO						11u

#define WL_MANAGE_TASK_PRIO						(7)	//无线管理任务优先级

#define ANGLE_SENSOR_TASK_PRIO					13u	//传感器数据采集任务优先级
#define ANGLE_SENSOR_MNG_TASK_PRIO				12u	//传感器数据管理任务优先级
#define UTIL_TIMER_TASK_PRIO                    15u

#define APPMNG_TASK_START_PRIO                  27u    /*系统开始任务 */
#ifdef IWDG_ENABLED
#define IWDG_TASK_PRIO							29	//独立看门狗任务	//注意：这是定义的最低优先级，如果继续降级，需要改变OS_LOWEST_PRIO
#endif
#define OS_TASK_TMR_PRIO                        (OS_LOWEST_PRIO - 2u)
/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/
#define UWBRX_TASK_STK_SIZE					256u
#define UWB_ANCHOR_TASK_STK_SIZE			512u

#define APPMNG_TASK_STK_SIZE				256u

#define APP_LIGHT_TASK_STK_SIZE				160u
#define APP_BEEP_TASK_STK_SIZE				128u
#define ANGLE_SENSOR_TASK_STK_SIZE			256u	//传感器数据采集任务堆栈大小
#define ANGLE_SENSOR_MNG_TASK_STK_SIZE		256u	//传感器数据管理任务堆栈大小
#ifdef IWDG_ENABLED
	#define IWDG_TASK_STK_SIZE					128u	//独立看门狗任务任务堆栈大小
#endif

#define WL_MANAGE_TASK_STK_SIZE				128u

#endif
