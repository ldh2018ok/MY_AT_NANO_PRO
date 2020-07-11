/*
*********************************************************************************************************
*
*   模块名称 : 头文件(For STM32F1)
*   文件名称 : my_include.h
*   版    本 : V1.0
*   说    明 : 这是所有应用模块自定义的总头文件，在每个C应用文件中包含此头文件。
*   修改记录 :
*       版本号          日期          作者       说明
*       V1.0    2020-07-9  ldh
*
*
*********************************************************************************************************
*/

#ifndef __my_include_H
#define __my_include_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "rtthread.h"
#include "myat_sever.h"
#include "my_timer.h"  
#include <string.h>     
#include <rthw.h>
     
#define RX1_REC_EVENT            (1<<0)              //接收到串口空闲中断事件位
#define RX1_PROCESS_EVENT        (1<<1)              //串口1数据处理事件位
#define AT_TIMEOUT_EVENT         (1<<2)              //AT模块指令返回数据超时事件位

#define ATSTATUS_SEND_EVENT      (1<<4)              //AT模块指令发送事件位
#define ATCMD_SEND_ACK_OK_EVENT  (1<<5)             //AT模块指令发送成功接收返回数据事件位

     
#ifdef __cplusplus
}
#endif

#endif

/* ********************************************END OF FILE***************************************** */



