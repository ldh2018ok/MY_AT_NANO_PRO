/*
*********************************************************************************************************
*
*   模块名称 : 头文件(For STM32F1)
*   文件名称 : usart.h
*   版    本 : V1.0
*   说    明 : 串口处理的头文件，包含于总头文件my_include.h中。
*   修改记录 :
*       版本号          日期          作者       说明
*       V1.0    2020-07-9  ldh
*
*
*********************************************************************************************************
*/

#ifndef __usart_H
#define __usart_H
#ifdef __cplusplus
 extern "C" {
#endif

/* 包含文件 ------------------------------------------------------------------*/
#include "main.h"

/* 常量定义 ------------------------------------------------------------------*/
#define USART1_DMA_REC_SIZE 512
#define USART1_PRO_SIZE   196

/* 全局变量及结构体声明 ------------------------------------------------------------------*/
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;   

typedef struct                                              //串口数据结构定义
  {  
    uint16_t  UsartDMARec_q_front;                          //DMA接收队列头指针
    uint16_t  UsartDMARec_q_rear;                           //DMA接收队列尾指针
    uint8_t UsartRecFlag;                                   //串口接收标志
    uint16_t UsartDMARecLen;                                //DMA接收到的数据长度
    uint16_t process_buff_p;                                //串口处理缓冲的数组指针
    uint8_t  *DMARecBuffer;                                 //DMA接收队列数组
    uint8_t  *process_buff;                                 //串口处理缓冲数组
  } Usart_recv_datatype;

extern Usart_recv_datatype Usart1_recv_type;                //串口1数据结构句柄声明
extern uint8_t UART1_DMARecBuffer[USART1_DMA_REC_SIZE];     //DMA循环接收缓冲数组
extern uint8_t UART1_process_buff[USART1_PRO_SIZE];         //串口1数据处理缓冲数组
extern struct rt_event rx1_event;                           //事件集控制块句柄声明

/* 函数声明 ------------------------------------------------------------------*/
void MX_UART4_Init(void);                                   //UART4初始化
void MX_USART1_UART_Init(void);                             //UART1初始化
void MX_DMA_Init(void);                                     //DMA初始化
void EnableUsart_IT(void);                                  //中断设置
int serial_thread_init(void);                               //串口数据接收线程初始化
void serial_entry(void *param);                             //串口数据接收线程入口函数

#ifdef __cplusplus
}
#endif
#endif


/* *****************************************************END OF FILE*******************************************/
