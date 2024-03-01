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

//����汾��Ϣ�ϱ�
#define VERSION_1    4
#define VERSION_2    0
#define VERSION_3    8

/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/
/*** 0��1��2��3Ԥ����uc/osϵͳ***************/
#define CANRX_TASK_PRIO							4u      /* can receive  task   */
#define CANTX_TASK_PRIO							10u     /* can transmit task   */
//#define UWBRX_TASK_PRIO						5u	    /*uwb�����������ȼ� */
//#define  UWB_DEBUG_TASK_PRIO					5u	    /*uwb�����������ȼ� */
#define UWB_ANCHOR_TASK_PRIO					5u
//#define UWB_TAG_TASK_PRIO						7u
//#define  TAG_REPORT_TAST_PRIO					8u
#define	 UWBAPP_TASK_PRIO						6u

#define APP_TASK_LIGHT_PRIO						9u

#define APP_TASK_BEEP_PRIO						11u

#define WL_MANAGE_TASK_PRIO						(7)	//���߹����������ȼ�

#define ANGLE_SENSOR_TASK_PRIO					13u	//���������ݲɼ��������ȼ�
#define ANGLE_SENSOR_MNG_TASK_PRIO				12u	//���������ݹ����������ȼ�
#define UTIL_TIMER_TASK_PRIO                    15u

#define APPMNG_TASK_START_PRIO                  27u    /*ϵͳ��ʼ���� */
#ifdef IWDG_ENABLED
#define IWDG_TASK_PRIO							29	//�������Ź�����	//ע�⣺���Ƕ����������ȼ������������������Ҫ�ı�OS_LOWEST_PRIO
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
#define ANGLE_SENSOR_TASK_STK_SIZE			256u	//���������ݲɼ������ջ��С
#define ANGLE_SENSOR_MNG_TASK_STK_SIZE		256u	//���������ݹ��������ջ��С
#ifdef IWDG_ENABLED
	#define IWDG_TASK_STK_SIZE					128u	//�������Ź����������ջ��С
#endif

#define WL_MANAGE_TASK_STK_SIZE				128u

#endif
