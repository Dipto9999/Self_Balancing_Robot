/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPEAKER_Pin GPIO_PIN_0
#define SPEAKER_GPIO_Port GPIOA
#define DISTANCE_SENSOR_FRONT_INPUT_CAPTURE_Pin GPIO_PIN_2
#define DISTANCE_SENSOR_FRONT_INPUT_CAPTURE_GPIO_Port GPIOA
#define DISTANCE_SENSOR_FRONT_PULSE_Pin GPIO_PIN_3
#define DISTANCE_SENSOR_FRONT_PULSE_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define SPI1_RST_Pin GPIO_PIN_0
#define SPI1_RST_GPIO_Port GPIOB
#define COLOR_SENSOR_STATUS_Pin GPIO_PIN_11
#define COLOR_SENSOR_STATUS_GPIO_Port GPIOA
#define RFID_STATUS_Pin GPIO_PIN_12
#define RFID_STATUS_GPIO_Port GPIOA
#define DISTANCE_SENSOR_FRONT_STATUS_Pin GPIO_PIN_15
#define DISTANCE_SENSOR_FRONT_STATUS_GPIO_Port GPIOA
#define DISTANCE_SENSOR_BACK_STATUS_Pin GPIO_PIN_3
#define DISTANCE_SENSOR_BACK_STATUS_GPIO_Port GPIOB
#define DISTANCE_SENSOR_BACK_INPUT_CAPTURE_Pin GPIO_PIN_4
#define DISTANCE_SENSOR_BACK_INPUT_CAPTURE_GPIO_Port GPIOB
#define DISTANCE_SENSOR_BACK_PULSE_Pin GPIO_PIN_5
#define DISTANCE_SENSOR_BACK_PULSE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
typedef enum
{
	DISTANCE_SENSOR_FRONT_ID,
	DISTANCE_SENSOR_BACK_ID,
	COLOR_SENSOR_ID,

} feature_id;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
