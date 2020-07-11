/*
*********************************************************************************************************
*
*   模块名称 : MAIN模块(For STM32F1)
*   文件名称 : main.h
*   版    本 : V1.0
*   说    明 : 这是main.c文件对应的头文件，被包含于总头文件my_include.h文件中
*   修改记录 :
*       版本号          日期          作者       说明
*       V1.0    2020-07-9  ldh
*
*
*********************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN ET */
int my_board_init(void);
int my_app_init(void);
/* USER CODE END ET */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/* ********************************************END OF FILE***************************************** */
