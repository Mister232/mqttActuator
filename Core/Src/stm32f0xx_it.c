/**
  ******************************************************************************
  * @file    stm32f0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

// Includes
#include "main.h"
#include "uart_com.h"
#include "stm32f0xx_it.h"

// External variables
extern TIM_HandleTypeDef htim3;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern UART_HandleTypeDef huart1;


/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  while (1)
  {
  }
}


/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
	while (1)
	{
	}
}


/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}


/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}


/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
}


/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel 2 and 3 interrupts.
  */
void DMA1_Channel2_3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_usart1_rx);
}


/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
	toggleLED = 1;
	HAL_TIM_IRQHandler(&htim3);
}


/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
	uint32_t tmp_flag = 0;
	uint32_t temp;

	tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE);

	if ((tmp_flag != RESET))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);
		temp = huart1.Instance->ISR;
		temp = huart1.Instance->RDR;
		HAL_UART_DMAStop(&huart1);
		temp = hdma_usart1_rx.Instance->CNDTR;
		ESP_RxLen = ESP_MAX_RECVLEN - temp;
		ESP_RecvEndFlag = 1;
	}

	// Enable interrupt again
	HAL_UART_Receive_DMA(&huart1, ESP_RxBUF, ESP_MAX_RECVLEN);

	HAL_UART_IRQHandler(&huart1);
}
