#ifndef __UWB_COMMON_H__
#define __UWB_COMMON_H__


#include "stdint.h"
#include "stdbool.h"
#include "can_app.h"
#include "deca_device_api.h"
#include "deca_regs.h"
//#include "filtering.h"

#define BEACON_FRAME_TYPE_IDX											7		//信标帧，帧子类型索引

#define FIRST_ADDR_OFF                            5
#define SECOND_ADDR_OFF                           7

#define FRAME_DEC_TAG_ADDR_IDX										5		//通讯声明帧，标签地址索引
#define FRAME_16BIT_ADDR_TYPE_IDX									9		//16Bit地址数据帧，帧类型索引


/* UWB microsecond (uus) 和 device time unit (dtu, 1/(499.2MHz*128)≈15.65ps) 换算系数.
 * 1 uus = 512 / 499.2 us 
 * 1 us  = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME											65536		//UUS与DWT芯片时间转换系数


#define FRAME_COMM_DEC_RX_TO_RESP_TX_DLY_UUS			300			//通讯声明帧接收完成到发送回复帧延迟时间
#define POLL_RX_TO_RESP_TX_DLY_UUS          			600	  //POLL接收完成到开始发送RESP延迟时间
#define RESP_TX_TO_FINAL_RX_DLY_UUS         			150   //RESP发送完成到开始接收FINAL延迟时间
#define FINAL_RX_TIMEOUT_UUS                			5000  	//FINAL接收超时时间


/*定位帧格式*/
#define FINAL_MSG_TS_LEN                      		5  //finally消息中，时间戳长度：5个字节
#define FINAL_MSG_POLL_TX_TS_IDX              		10  //finally消息中，POLL发送时间戳索引
#define FINAL_MSG_RESP_RX_TS_IDX              		(FINAL_MSG_POLL_TX_TS_IDX + FINAL_MSG_TS_LEN)  //finally消息中，RESP发送时间戳索引
#define FINAL_MSG_FINAL_TX_TS_IDX             		(FINAL_MSG_RESP_RX_TS_IDX +  FINAL_MSG_TS_LEN) //finally消息中，FINAL发送时间戳索引


typedef union 
{
	struct {
		uint16_t frameType:3; //帧类型
		uint16_t  security:1;  //加密
		uint16_t 	pending:1;   //uwb 不使用
		uint16_t  ack:1;        //是否需要 ack
		uint16_t  panIdCp:1;  //PAN ID 压缩模式
		uint16_t  reserve:3;     //保留
		uint16_t  destAddrMode:2; //目的地址帧id长度
		uint16_t  frameVer:2;     
		uint16_t  sorcAddrMode:2;  //源地址帧id长度
	}FrameConBit;
	uint16_t  frameControl;
}FrameConField;	


//typedef struct 
//{
//		union {
//		struct {
//			uint16_t frameType:3; //帧类型
//			uint16_t  security:1;  //加密
//			uint16_t 	pending:1;   //uwb 不使用
//			uint16_t  ack:1;        //是否需要 ack
//			uint16_t  panIdCp:1;  //PAN ID 压缩模式
//			uint16_t  reserve:3;     //保留
//			uint16_t  destAddrMode:2; //
//			uint16_t  frameVer:2;
//			uint16_t  sorcAddrMode:2;  
//		}FrameConBit;
//		uint16_t  frameControl;
//	}FrameConField;	
//}

/*IEEE 802.15.4帧类型定义*/

typedef enum{
	BEACON_FRAME = 0,
	DATA_FRAME,
	ACK_FRAME,
	MAC_CMD_FRAME,
	RES_FRAME
}IEEE_FRAME_TYPE;
	
/*帧类型定义*/

typedef enum{
	FRAME_POLL = 11,														//Poll帧
	FRAME_RESP,																// 12 Resp帧
	FRAME_FINAL,															//13	//Final帧
	FRAME_DIST_INFO,													//14	//距离信息帧
	FRAME_COMM_DEC,														//15	//Tag通讯声明帧
	FRAME_COMM_DEC_RESP												//16	//Anc通讯声明回复帧
}LOCL_FRAME_TYPE;


/*帧id长度定义*/
typedef enum{
	ID_LENGTH_NO = 0,
	ID_LENGTH_RES,
	ID_LENGTH_16,
	ID_LENGTH_64
}FRAME_ID_LENGTH;



void SetFirstAddr(uint8_t * dest, uint16_t addr);


void SetSecondAddr(uint8_t * dest, uint16_t addr);

void FinalMsgGetTS_64(const uint8_t *ts_field, uint64_t *ts);
void FinalMsgSetTS(uint8 *ts_field, uint64_t ts);

#endif /*__UWB_COMMON_H__*/
