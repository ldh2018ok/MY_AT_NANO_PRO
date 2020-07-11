/*
*********************************************************************************************************
*
*   模块名称 : 串口模块
*   文件名称 : usart.c
*   版    本 : V1.0
*   说    明 : 串口程序模块
*              主要功能：
*                1、实现RTT控制台的串口输出及捕获输入函数。
*                2、实现AT模块的串口接收及发送硬件底层实现：DMA接收，查询方式发送。
*                3、通过事件方式与线程通信。
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

/* 自定义数组及变量----------------------------------------------------------------*/
uint8_t UART1_DMARecBuffer[USART1_DMA_REC_SIZE];        //DMA串口接收数组定义
uint8_t UART1_process_buff[USART1_PRO_SIZE];            //串口接收处理缓冲区定义
Usart_recv_datatype Usart1_recv_type={0,0,0,0,0,UART1_DMARecBuffer,UART1_process_buff};  //串口数据结构体定义及初始化
struct rt_event rx1_event;                              //事件控制块句柄定义
static char serial_stack[1024]={0};                     //串口接收处理线程栈
static struct rt_thread serial;                         //串口接收处理线程控制块句柄定义

/* 串口及DMA硬件操作句柄定义----------------------------------------------------------------*/
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* UART4串口初始化，该端口用于RTT控制台操作----------------------------------------------------------------*/
void MX_UART4_Init(void)
{
    huart4.Instance = UART4;
    huart4.Init.BaudRate = 115200;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart4) != HAL_OK)
    {
        Error_Handler();
    }
}

/* UART1串口初始化，该端口用于连接AT模块----------------------------------------------------------------*/
void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

/* 串口的MSP初始化（与MCU相关的初始化如：引脚，中断，DMA，等）------------------------------------------------*/
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(uartHandle->Instance==UART4)
    {
        //使能UART4及相应GPIO外设时钟
        __HAL_RCC_UART4_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        //UART4的GPIO设置：PC10--> UART4_TX ，PC11---> UART4_RX
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }
    else if(uartHandle->Instance==USART1)
    {
        //使能UART1及相应GPIO外设时钟
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        //UART1的GPIO设置：PA9--> UART1_TX ，PA10---> UART1_RX

        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        //UART1的RX采用DMA方式接收，初始化相应DMA设置
        hdma_usart1_rx.Instance = DMA1_Channel5;
        hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
        hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
        if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
        {
            Error_Handler();
        }
        //串口接收外设与DMA关联设置
        __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

        //串口中断设置：优先级，中断使能
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);

    }
}

/* 串口的MSPDeinit初始化--------------------------------------------------------------------*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

    if(uartHandle->Instance==UART4)
    {
        //失能UART4外设时钟
        __HAL_RCC_UART4_CLK_DISABLE();
       //失能 UART4的GPIO
         HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11);

    }
    else if(uartHandle->Instance==USART1)
    {
        //失能UART1外设时钟
        __HAL_RCC_USART1_CLK_DISABLE();
        //失能 UART1的GPIO
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
        //失能 UART1的RX接收DMA
        HAL_DMA_DeInit(uartHandle->hdmarx);
        //失能 UART1的中断
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
} 

/* 串口接收线程的入口函数-----------------------------------------------------------------*/
void serial_entry(void *param)
{
    static rt_err_t result1 = RT_EOK;
    static rt_uint32_t e=0;
    static uint16_t process_p=0;    //接收处理缓冲的数组指针
    while (1)
        {
            //等待串口空闲中断接收事件
            result1=rt_event_recv(&rx1_event, RX1_REC_EVENT,
                              RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                              RT_WAITING_FOREVER, &e) ;
            //接收到串口空闲中断接收事件
            if(result1==RT_EOK)
            {
                //如果DMA队列不为空则读取数据至串口处理缓冲
                while(Usart1_recv_type.UsartDMARec_q_front!=Usart1_recv_type.UsartDMARec_q_rear)
                {
                    Usart1_recv_type.process_buff[process_p++]=Usart1_recv_type.DMARecBuffer[Usart1_recv_type.UsartDMARec_q_rear];
                    Usart1_recv_type.UsartDMARec_q_rear=(Usart1_recv_type.UsartDMARec_q_rear+1)%USART1_DMA_REC_SIZE;
                    //如串口处理缓冲满则退出数据读取
                    if(process_p==USART1_PRO_SIZE)
                        break;
                }

                Usart1_recv_type.process_buff_p=process_p;           //记录所读取数据字节数
                process_p=0;                                         //数组指针清零，准备下一次接收
                //发送新数据接收完成事件
                if(rt_event_send(&rx1_event, RX1_PROCESS_EVENT)!=RT_EOK)
                {
                    rt_kprintf("rx1 receive event send failed!");//事件发送错误打印信息输出 ，测试
                }
                //打印输出所接收到的数据，测试
                HAL_UART_Transmit(&huart4, Usart1_recv_type.process_buff, Usart1_recv_type.process_buff_p, 0xfff);
                //rt_kprintf("%s\r\n",(char *)Usart1_recv_type.process_buff);

            }

        }
}

/* 串口接收线程初始化---------------------------------------------------------------------*/
int serial_thread_init(void)
{
    rt_err_t result = RT_EOK;
    //创建事件集（串口接收，数据处理，超时，发送）
    result = rt_event_init(&rx1_event, "rx1_event", RT_IPC_FLAG_FIFO);
    if (result == RT_EOK)
    {
        rt_kprintf("init rx1_event ok!\n");    //事件集创建成功打印信息输出 ，测试
    }

    //创建串口接收线程）
    rt_thread_init(&serial,
                       "serial",
                       serial_entry,
                       RT_NULL,
                       &serial_stack[0],
                       sizeof(serial_stack),
                       3, 20);
    rt_thread_startup(&serial);
    return RT_EOK;
}

/* 外设中断的初始化，将在系统板级初始化中被调用------------------------------------------------*/
void EnableUsart_IT(void)
{
    //__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);  //开启串口接收中断
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);    //开启串口1空闲接收中断
     HAL_UART_Receive_DMA(&huart1,Usart1_recv_type.DMARecBuffer,USART1_DMA_REC_SIZE);    //使能DMA接收
     __HAL_UART_CLEAR_IDLEFLAG(&huart1);             //清除串口1空闲中断标志位
}

/* RTT控制台输出端口函数：函数名不能更改！------------------------------------------------*/
void rt_hw_console_output(const char *str)
{

  char a = '\r';
  while(*str!='\0')
  {
    if(*str == '\n')
    {
      HAL_UART_Transmit(&huart4,(uint8_t *)&a,1,0xff);
    }
    HAL_UART_Transmit(&huart4,(uint8_t *)str++,1,0xff);
  }
}

/* RTT控制台输入捕获函数：函数名不能更改！------------------------------------------------*/
char rt_hw_console_getchar(void)
{
  int ch = -1;
    
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE) != RESET)
    {
        ch = huart4.Instance->DR & 0xff;
    }
    else
    {
        if(__HAL_UART_GET_FLAG(&huart4, UART_FLAG_ORE) != RESET)
        {
            __HAL_UART_CLEAR_OREFLAG(&huart4);
        }
        rt_thread_mdelay(10);
    }
  return ch;
} 


/* *************************************************END OF FILE*********************************************** */
