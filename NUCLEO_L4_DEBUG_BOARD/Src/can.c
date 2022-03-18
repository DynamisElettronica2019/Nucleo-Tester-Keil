/**
  ******************************************************************************
  * File Name          : CAN.c
  * Description        : This file provides code for the configuration
  *                      of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
static CAN_FilterTypeDef CAN_Filter_Config;
static CAN_TxHeaderTypeDef nucleo_L4_Packet_Header;
static CAN_RxHeaderTypeDef CAN_Received_0_Message_Header;
static CAN_RxHeaderTypeDef CAN_Received_1_Message_Header;
static uint8_t CAN_Received_0_Message_Data[8];
static uint8_t CAN_Received_1_Message_Data[8];
static uint8_t nucleo_L4_Packet_Data[8];
extern char receivedOk;
extern uint32_t emptyMailboxes;

//indirizzo CAN nucleo L4 (provvisorio per test)
#define NUCLEO_L4_ID ((uint16_t)(500))

/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 4;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();
  
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**CAN1 GPIO Configuration    
    PD0     ------> CAN1_RX
    PD1     ------> CAN1_TX 
    */
    GPIO_InitStruct.Pin = CAN1_RX_Pin|CAN1_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();
  
    /**CAN1 GPIO Configuration    
    PD0     ------> CAN1_RX
    PD1     ------> CAN1_TX 
    */
    HAL_GPIO_DeInit(GPIOD, CAN1_RX_Pin|CAN1_TX_Pin);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
extern void CAN1_Start(void)
{
	CAN1_FilterSetup();
	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY);
	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO1_MSG_PENDING);
	//HAL_CAN_ActivateNotification(&hcan1, );
	HAL_CAN_Start(&hcan1);
	
}

void CAN1_FilterSetup(void)
{
	
 
	/*
  Mittente: NUCLEO F7
	IDs = 0011111xxxx
	MASK = 0x7F0 	(0111 1111 0000) //First 0 is not considered because of bit shift
	Filter = 0x1F0 	(0001 1111 0000)
	*/
  CAN_Filter_Config.FilterBank = 0;
  CAN_Filter_Config.FilterMode = CAN_FILTERMODE_IDMASK;
  CAN_Filter_Config.FilterScale = CAN_FILTERSCALE_32BIT;
	CAN_Filter_Config.FilterIdHigh = (0x1F0 << 5);
  CAN_Filter_Config.FilterIdLow = 0x0000;
  CAN_Filter_Config.FilterMaskIdHigh = (0x7F0  << 5);
  CAN_Filter_Config.FilterMaskIdLow = 0x0000;
	CAN_Filter_Config.FilterFIFOAssignment = CAN_RX_FIFO0;
  CAN_Filter_Config.FilterActivation = ENABLE;	
  CAN_Filter_Config.SlaveStartFilterBank = 14;
	
	HAL_CAN_ConfigFilter(&hcan1, &CAN_Filter_Config);	
	
}

extern void CAN1_Send_Nucleo_L4_Packet(void)
{
	uint32_t nucleo_L4_Packet_Mailbox;
	nucleo_L4_Packet_Header.StdId = NUCLEO_L4_ID;
  nucleo_L4_Packet_Header.RTR = CAN_RTR_DATA;
  nucleo_L4_Packet_Header.IDE = CAN_ID_STD;
  nucleo_L4_Packet_Header.DLC = 8;
  nucleo_L4_Packet_Header.TransmitGlobalTime = DISABLE;
  nucleo_L4_Packet_Data[0] = 5;
  nucleo_L4_Packet_Data[1] = 0;
  nucleo_L4_Packet_Data[2] = 10;
  nucleo_L4_Packet_Data[3] = 0;
  nucleo_L4_Packet_Data[4] = 0;
  nucleo_L4_Packet_Data[5] = 6;
	nucleo_L4_Packet_Data[6] = 0;
  nucleo_L4_Packet_Data[7] = 5;
	HAL_CAN_AddTxMessage(&hcan1, &nucleo_L4_Packet_Header, nucleo_L4_Packet_Data, &nucleo_L4_Packet_Mailbox);
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
	
	
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &CAN_Received_0_Message_Header, CAN_Received_0_Message_Data); 
	if(CAN_Received_0_Message_Data[0] == 6)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
		receivedOk = 1;
	}
}

void CAN1_UnPacking(void)
{
	
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
