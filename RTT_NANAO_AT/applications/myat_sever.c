/*
*********************************************************************************************************
*
*   模块名称 : AT程序模块
*   文件名称 : myat_sever.c
*   版    本 : V1.0
*   说    明 : AT程序模块
*              主要功能：
*                1、处理AT模块的相关AT指令发送及接收返回数据
*                2、采用接收处理及超时重发处理线程、发送指令线程、软件定时器
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT查看打印信息，波特率115200，数据位8，奇偶校验位无，停止位1。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
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

uint8_t CurrentRty=0;                                       //重发次数
teATCmdNum ATRecCmdNum;                                     //接收数据指令枚举序号״̬
teATCmdNum ATNextCmdNum;                                    //下条AT指令枚举序号
teATCmdNum ATCurrentCmdNum;                                 //当前AT指令枚举序号

char at_pro_stack[1024]={0};                                //AT指令接收数据处理线程栈
struct rt_thread at_process;                                //AT指令接收数据处理线程控制块句柄
char at_send_stack[1024]={0};                               //AT指令发送数据线程栈
struct rt_thread at_send_thread;                            //AT指令发送数据线程控制块句柄
ALIGN(RT_ALIGN_SIZE)

tsATCmds ATCmds[] =      //AT指令结构列表：重发次数，指令字符串，返回包关键字符，AT状态，超时时间（10ms单位，以timer2设置为准）
{
        //以下指令适用于LY蓝牙模块，如使用其它型号，相应的指令格式要更改
        {3,"AT+ROLE\r\n","ROLE:",NO_REC,200},           //查主从模式
        {3,"AT+VERS\r\n","V",NO_REC,200},               //查固件版本
        {3,"AT+GNAM\r\n","NAME:",NO_REC,200},           //查设备名称
        {3,"AT+GURT\r\n","UART:",NO_REC,200},           //查波特率
        {3,"AT+GSTA\r\n","ADVE",NO_REC,200},            //查蓝牙状态：是否广播开启״̬
        {3,"AT+GPWD\r\n","Pair code",NO_REC,200},       //查配对密码
        {3,"AT+CONB\r\n","OK",NO_REC,200},              //断开蓝牙连接
        {3,"AT+ADVL\r\n ","ADVERTISING_ON",NO_REC,200}, //开启广播
        {3,"AT+ADVB\r\n","ADVERTISING_OFF",NO_REC,200}, //关闭广播
        {3,"AT+REST\r\n","OK",NO_REC,200},              //模块复位
        {3,"AT+TNOCMD\r\n","OK",NO_REC,200},            //透传设置自定义
        {3,"AT+TNOCMD1\r\n","OK",NO_REC,200},           //补充定义指令

    /*{"AT\r\n","OK",200,NO_REC,100},                                        //
    {"ATE0\r\n","OK",200,NO_REC,100},                                      //
    {"AT+GMR\r\n","ME3616",200,NO_REC,3},                                  //
    {"ATI\r\n","OK",200,NO_REC,3},                                         //
    {"AT+ZCONTLED=1\r\n","OK",1000,NO_REC,2},                              //

    {"AT+ZSLR?\r\n","+ZSLR:1",2000,NO_REC,2},                              //
    {"AT+ZSLR=1\r\n","OK",1000,NO_REC,2},                                  //
    {"AT+CPSMS?\r\n","+CPSM",2000,NO_REC,2},                               //
    {"AT+CPSMS=1,,,\"00011000\",\"00001010\"\r\n","OK",2000,NO_REC,2},     //

    {"AT+CSQ\r\n","+CSQ:",200,NO_REC,3},                                    //
    {"AT+CEREG?\r\n","+CEREG: 0,1",200,NO_REC,255},                        //

    {"AT+ECOAPNEW=106.13.150.28,5683,1\r\n","+ECOAPNEW:",3000,NO_REC,3},    //
    {"AT+CSQ\r\n","+CSQ:",200,NO_REC,3},                                    //ֵ
    {"AT+CCLK?\r\n","GMT",200,NO_REC,2},                                   //
    {"AT+ECOAPSEND=1,","COAPNMI:",10000,NO_REC,5},                         //
    {"AT+ECOAPDEL=1\r\n","OK",2000,NO_REC,3},                              //
    {"AT+ZRST","OK",2000,NO_REC,3},                                        // */
};

/* AT数据接收处理线程入口函数----------------------------------------------------------*/
void at_process_entry(void *param)
{
    static rt_err_t result1 = RT_EOK;
    static rt_uint32_t e=0;
    while (1)
    {
        //等待接收数据接收完整包事件
        result1=rt_event_recv(&rx1_event, RX1_PROCESS_EVENT|AT_TIMEOUT_EVENT,
                                  RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,
                                  RT_WAITING_FOREVER, &e) ;
        if(result1==RT_EOK)                                     //成功接收
        {
           if(e&(rt_uint32_t)(RX1_PROCESS_EVENT))               //如果是串口数据包事件
           {
                
                Rec_WaitAT();                                   //处理数据
           }
           else if(e&(rt_uint32_t)(AT_TIMEOUT_EVENT))           //如果是超时事件
           {
                if(CurrentRty > 0)  //重发
                {
                    CurrentRty--;
                    rt_kprintf("@@ now:%d,retry time:%d\r\n",ATCurrentCmdNum,CurrentRty);//重发状态打印输出
                    ATNextCmdNum = ATCurrentCmdNum;  //赋值区分是否重发
                    at_timeout_cnt=500;         //超时设置5s
                    rt_timer_start(&timer2);    //开启超时定时器timer2
                    if(rt_event_send(&rx1_event, ATSTATUS_SEND_EVENT)==RT_EOK) //发送重发事件启动指令重发
                        rt_kprintf("atcmd resent!");    //

                }
                else  //重发次数用完
                {

                    ATSend(AT_REST);        //模块复位

                }
           
           }
        
        }
        
    }
}

/* AT指令发送线程入口函数----------------------------------------------------------*/
void at_send_thread_entry(void *param)
{
    static rt_err_t result1 = RT_EOK;
    static rt_uint32_t e=0;
    while (1)
    {
        //等待发送事件
        result1=rt_event_recv(&rx1_event, ATSTATUS_SEND_EVENT,
                                  RT_EVENT_FLAG_AND|RT_EVENT_FLAG_CLEAR,
                                  RT_WAITING_FOREVER, &e) ;
        if(result1==RT_EOK)     //发送事件接收成功
        {
            if(ATCurrentCmdNum != ATNextCmdNum)      //判断重发状态
            {
                CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum;   //重发次数赋值
            }
            //发送当前指令
            rt_enter_critical();
            HAL_UART_Transmit(&huart1,(uint8_t*)ATCmds[ATCurrentCmdNum].ATSendStr,strlen(ATCmds[ATCurrentCmdNum].ATSendStr),0xff);
            //打印输出发送的指令，测试
            rt_kprintf("@@Send:%s\r\n",ATCmds[ATCurrentCmdNum].ATSendStr);
            rt_exit_critical();
        }

    }
}

/* AT模块处理线程初始化----------------------------------------------------------*/

 int AT_PROCESS_TASK_init(void)
{
     //接收数据线程初始化，静态模式
    rt_thread_init(&at_process,
                       "at_process",
                       at_process_entry,
                       RT_NULL,
                       &at_pro_stack[0],
                       sizeof(at_pro_stack),
                       4, 20);
    //启动线程
    rt_thread_startup(&at_process);

    //AT模块发送线程初始化
    rt_thread_init(&at_send_thread,
                       "at_send_thread",
                       at_send_thread_entry,
                       RT_NULL,
                       &at_send_stack[0],
                       sizeof(at_send_stack),
                       5, 10);
    //启动线程
    rt_thread_startup(&at_send_thread);                       
    return RT_EOK;
}

 /* AT指令发送函数，形参为AT指令列表枚举的序号----------------------------------------------------------*/
void ATSend(teATCmdNum ATCmdNum)
{
    //发送指令参数赋值
    ATCurrentCmdNum=ATCmdNum;
    ATNextCmdNum=AT_TNOCMD;
    ATRecCmdNum=ATCmdNum;
    //超时参数赋值并启动定时器
    at_timeout_cnt=500;         //time delay=5s
    rt_timer_start(&timer2);
    //启动发送事件
    rt_enter_critical();
    if(rt_event_send(&rx1_event, ATSTATUS_SEND_EVENT)==RT_EOK)
    {
        rt_kprintf("atcmd sent!");//事件启动成功状态打印输出
    }
    rt_exit_critical();
}

/* AT模块数据等待接收并处理----------------------------------------------------------*/
void Rec_WaitAT(void)
{
    //判断接收字符串与相对指令的返回包关键字符的匹配是否一致
    if (strstr((const char*) Usart1_recv_type.process_buff, ATCmds[ATRecCmdNum].ATRecStr) != NULL)
    {
        at_timeout_cnt=0;
        switch (ATCurrentCmdNum)
        {

            case AT_ROLE: //
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                rt_memset(Usart1_recv_type.process_buff, 0, USART1_PRO_SIZE);
                break;
            case AT_VERS: //
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                rt_memset(Usart1_recv_type.process_buff, 0, USART1_PRO_SIZE);
                break;
            case AT_GNAM: //
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                rt_memset(Usart1_recv_type.process_buff, 0, USART1_PRO_SIZE);
                break;
            case AT_GURT: //
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                rt_memset(Usart1_recv_type.process_buff, 0, USART1_PRO_SIZE);
                break;
            case AT_GSTA: //״̬
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                rt_memset(Usart1_recv_type.process_buff, 0, USART1_PRO_SIZE);
                break;
            case AT_GPWD: //
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                rt_memset(Usart1_recv_type.process_buff, 0, USART1_PRO_SIZE);
                break;
            case AT_REST: //
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                rt_memset(Usart1_recv_type.process_buff, 0, USART1_PRO_SIZE);
                break;
            case AT_ADVL: //
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                rt_memset(Usart1_recv_type.process_buff, 0, USART1_PRO_SIZE);
                break;
            case AT_ADVB: //
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                rt_memset(Usart1_recv_type.process_buff, 0, USART1_PRO_SIZE);
                break;
            case AT_TNOCMD: //
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                break;
            default:
                rt_kprintf("cmd send: %d!  cmd ack: %s\r\n",ATCurrentCmdNum,(const char*) Usart1_recv_type.process_buff);
                break;
        }
        //数据接收处理成功，发送相应成功事件
        rt_event_send(&rx1_event, ATCMD_SEND_ACK_OK_EVENT);
    }
    //匹配错误，打印输出错误信息，测试
    else
    {
        rt_kprintf("AT cmd match failed!");
    }

}

/* AT模块指令列表测试的控制台命令---------------------------------------------------------*/
int at_comtest(void)
{
    static rt_err_t result1 = RT_EOK;
    static rt_uint32_t e=0;
    teATCmdNum AT_SEND_CMD_NUM=AT_ROLE;
    char kk=0;
    for (kk = 0; kk < AT_TNOCMD-1; ++kk)
    {
        ATSend(AT_SEND_CMD_NUM);
        result1=rt_event_recv(&rx1_event, ATCMD_SEND_ACK_OK_EVENT,
                                  RT_EVENT_FLAG_AND|RT_EVENT_FLAG_CLEAR,
                                  RT_WAITING_FOREVER, &e) ;
        if(result1==RT_EOK)
        {
            AT_SEND_CMD_NUM++;
        }
    }
    return 0;
}
MSH_CMD_EXPORT(at_comtest,TEST THE AT CMD LIST);

/* 在字符串中根据给定头尾标识符，找出相应字符串---------------------------------------------------------*/
/*
int Find_string(char *pcBuf,char *left,char *right, char *pcRes)
{
    char *pcBegin = NULL;
    char *pcEnd = NULL;
    pcBegin = strstr(pcBuf, left);//ȡ���������
    pcEnd = strstr(pcBegin+strlen(left), right);//ɨ���ұ߱�ʶ
    if(pcBegin == NULL || pcEnd == NULL || pcBegin > pcEnd)
    {
        rt_kprintf("string name not found!\n");
        return 0;
    }
    else
    {
        pcBegin += strlen(left);
        memcpy(pcRes, pcBegin, pcEnd-pcBegin);
        return 1;
    }
}
*/

/* 设置AT模块为AT指令工作模式的控制台命令---------------------------------------------------------*/
int AT_CMD_MODE(void)
{
    HAL_GPIO_WritePin(BLE_TRANS_MODE_PORT, BLE_TRANS_MODE_PIN, GPIO_PIN_SET);   //1---AT指令模式;0---透传模式
    HAL_GPIO_WritePin(BLE_UART_EN_PORT, BLE_UART_EN_PIN, GPIO_PIN_SET);         //1----使能串口; 0----禁止串口
    rt_thread_mdelay(20);
    return 0;
}
MSH_CMD_EXPORT(AT_CMD_MODE, enable ATcomand mode);

/* 设置AT模块透传模式的控制台命令---------------------------------------------------------*/
int AT_DIRECT_MODE(void)
{
    HAL_GPIO_WritePin(BLE_TRANS_MODE_PORT, BLE_TRANS_MODE_PIN, GPIO_PIN_RESET); //1---AT指令模式;0---透传模式
    HAL_GPIO_WritePin(BLE_UART_EN_PORT, BLE_UART_EN_PIN, GPIO_PIN_SET);         //1----使能串口; 0----禁止串口
    rt_thread_mdelay(20);
    return 0;
}
MSH_CMD_EXPORT(AT_DIRECT_MODE, enable ATdirect transmit mode);

/* AT模块参数初始化，将在APP初始化中调用---------------------------------------------------------*/
int AT_Init(void)
{
    AT_CMD_MODE();                  //AT指令工作模式
    ATCurrentCmdNum = AT_ROLE;      //非重发状态设置
    ATNextCmdNum=AT_TNOCMD1;
    at_timeout_cnt=0;               //变量初始化


    return 0;
}



