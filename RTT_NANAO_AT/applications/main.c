
/*
*********************************************************************************************************
*
*   模块名称 : 主程序模块
*   文件名称 : main.c
*   版    本 : V1.0
*   说    明 : 主程序
*              主要功能：
*                1、初始化应用程序相应参数
*                2、初始化并启动线程
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "my_include.h"
/* USER CODE END Includes */
#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* main loop ----------------------------------------------------------------*/
int main(void)
{

  /* USER CODE BEGIN SysInit */
    rt_thread_mdelay(100);
    my_app_init();                      //初始化应用参数及线程
  /* USER CODE END SysInit */

   /* USER CODE BEGIN 2 */
	rt_kprintf("Nano test startup!\r\n");   //启动信息打印输出
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
      rt_thread_mdelay(1000);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/* SystemClock_Config ---------------------------------------------------------*/
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

// Initializes the CPU, AHB and APB busses clocks

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
// Initializes the CPU, AHB and APB busses clocks

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* hardware level board init: will be called in common.c before system startup-----------------------------------------*/
int my_board_init(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
    MX_DMA_Init();
    MX_UART4_Init();
    MX_USART1_UART_Init();
    rt_memset(Usart1_recv_type.DMARecBuffer, 0, USART1_DMA_REC_SIZE);
    rt_memset(Usart1_recv_type.process_buff, 0, USART1_PRO_SIZE);
    CurrentRty=0;
    at_timeout_cnt=0;

	return 0;
}

/* app level init: will be called in main.c before app startup-----------------------------------------*/
int my_app_init(void)
{
    rt_base_t level;
    level=rt_hw_interrupt_disable();
    AT_Init();
    serial_thread_init();
    AT_PROCESS_TASK_init();
    timer_sof_init();
    EnableUsart_IT();
    rt_hw_interrupt_enable(level);
    return 0;
}
/* USER CODE END 4 */

/* Error_Handler-----------------------------------------*/
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}


/* ****************************END OF FILE***************************************** */
