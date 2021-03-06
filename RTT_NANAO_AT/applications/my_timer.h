/*
*********************************************************************************************************
*
*   模块名称 : 头文件(For STM32F1)
*   文件名称 : my_timer.h
*   版    本 : V1.0
*   说    明 : 软件定时器的头文件，包含于总头文件my_include.h中。
*   修改记录 :
*       版本号          日期          作者       说明
*       V1.0    2020-07-9  ldh
*
*
*********************************************************************************************************
*/
#ifndef _MY_TIMER_H_
#define _MY_TIMER_H_

extern struct rt_timer  timer1;     //软件定时器控制块句柄
extern struct rt_timer  timer2;

extern int at_timeout_cnt ;     //超时计数器，以定时器定时周期为单位

int timer_sof_init(void);       //软件定时器初始化函数

#endif
