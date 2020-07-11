/*
*********************************************************************************************************
*
*   模块名称 : AT处理模块(For STM32F1)
*   文件名称 : myat_sever.h
*   版    本 : V1.0
*   说    明 : 这是myat_seve.c文件对应的头文件，被包含于总头文件my_include.h文件中
*   修改记录 :
*       版本号          日期          作者       说明
*       V1.0    2020-07-9  ldh
*
*
*********************************************************************************************************
*/
#ifndef MYAT_SEVER_H
#define MYAT_SEVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* AT数据接收状态：成功，超时，未接收---------------------------------------------------------------*/
typedef enum
{
    SUCCESS_REC = 0,
    TIME_OUT,
    NO_REC
}te_AT_Status;

/* AT指令数据结构----------------------------------------------------------------------------------*/
typedef struct
{
    uint8_t RtyNum;         //超时重发次数
    char *ATSendStr;        //发送的数据指针
    char *ATRecStr;         //接收的数据指针
    te_AT_Status ATStatus;  //AT数据接收状态״̬
    uint16_t TimeOut;       //超时时长，以超时定时器的中断时间设定为单位：10ms
}tsATCmds;

extern tsATCmds ATCmds[];   //AT指令列表全局声明
extern uint8_t CurrentRty;  //重发次数
/* AT指令列表枚举----------------------------------------------------------------------------------*/
typedef enum
{
    AT_ROLE=0,      //查主从模式
    AT_VERS,        //查固件版本
    AT_GNAM,        //查设备名称
    AT_GURT,        //查波特率
    AT_GSTA,        //查蓝牙状态：是否广播开启״̬
    AT_GPWD,        //查配对密码
    AT_CONB,        //断开蓝牙连接
    AT_ADVL,        //开启广播
    AT_ADVB,        //关闭广播
    AT_REST,        //模块复位
    AT_TNOCMD,      //透传设置自定义
    AT_TNOCMD1,     //补充定义指令

 /*   AT_GMR,       //
    AT_I,           //
    AT_ZCONTLED,    //
    AT_ZSLR,        //
    AT_ZSLR_1,      //
    AT_CPSMS,       //
    AT_CPSMS_1,     //
    AT_CSQ,         //
    AT_CEREG,       //
    AT_ECOAPNEW,    //
    AT_CSQ_2,       //
    AT_CCLK,        //
    AT_ECOAPSEND,   //
    AT_ECOAPDEL,    //
    AT_ZRST,*/

}teATCmdNum;

extern teATCmdNum ATNextCmdNum;         //下个指令
extern teATCmdNum ATCurrentCmdNum;      // 当前指令

int AT_CMD_MODE(void);                  //AT指令模式设置函数
int AT_DIRECT_MODE(void);               //透传模式设置函数
int AT_Init(void);                      //AT模块初始化函数
void Rec_WaitAT(void);                  //等待接收处理AT模块数据
void ATSend(teATCmdNum ATCmdNum);       //AT指令发送函数（必须在命令列表中）
int AT_PROCESS_TASK_init(void);         //AT模块数据处理线程初始化
void at_process_entry(void *param);     //AT模块数据处理线程入口函数
void at_send_thread_entry(void *param); //AT指令发送线程入口函数
//int Find_string(char *pcBuf,char *left,char *right, char *pcRes);

#ifdef __cplusplus
}
#endif

#endif
