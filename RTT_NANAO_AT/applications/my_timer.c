/*
*********************************************************************************************************
*
*   模块名称 : 软件定时器模块
*   文件名称 : my_timer.c
*   版    本 : V1.0
*   说    明 : 主程序
*              主要功能：
*                1、软件定时器初始化及相应的定时器中断处理
*
*   修改记录 :
*       版本号            日期         作者
*       V1.0    2020-07-9   ldh
*
*    硬件平台为STM32F103VCT8
*   本程序在rt-thread nano3.1.3  RTT STUDIO V1.1.2平台上编译执行通过。
*
*********************************************************************************************************
*/

#include "my_include.h"


/* 定义软件定时器参数 ----------------------------------------------------------*/
struct rt_timer  timer1;
struct rt_timer  timer2;

int at_timeout_cnt=0;           //超时计数器
static int cnt=0;

/* 软件定时器1中断处理 ----------------------------------------------------------*/
void timeout1(void *parameter)

{
    rt_kprintf("Timer1 is timeout! %d\n",cnt);
    if (cnt++ >= 10)    //计数10次后结束定时器1.测试用
    {
        rt_timer_stop(&timer1);
        rt_kprintf("Timer1 is stopped! \n");
    }

}

/* 软件定时器2中断处理 ----------------------------------------------------------*/

void timeout2(void *parameter)

{
    if(at_timeout_cnt)      //计数延时递减
    {
        at_timeout_cnt--;
        if(at_timeout_cnt==0)   //超时确认
        {
            rt_event_send(&rx1_event,AT_TIMEOUT_EVENT); //发送超时事件
            rt_timer_stop(&timer2);                     //停止软件定时器2
        }
    }

}

/* 软件定时器定义及初始化 ----------------------------------------------------------*/
int timer_sof_init(void)
{
    //定时器1初始化 100ms定时
    rt_timer_init(&timer1, "timer1", timeout1, RT_NULL, 100, RT_TIMER_FLAG_PERIODIC);
    rt_timer_start(&timer1);

    //定时器2初始化  10ms定时，作为延时计数，在使用时启动
    rt_timer_init(&timer2, "timer2", timeout2, RT_NULL, 10, RT_TIMER_FLAG_PERIODIC);
    rt_timer_stop(&timer2);
    return 0;
}

