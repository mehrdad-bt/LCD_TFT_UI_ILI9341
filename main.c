/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "buttons.h"
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

	typedef enum {
		PAGE_MAIN,
		PAGE_SECOND
	} Page_t;
	
	
	typedef struct {
		uint16_t x;
		uint16_t y;
		uint16_t width;
		uint16_t height;
		const uint16_t* normal_img;
		const uint16_t* pressed_img;
		uint8_t is_pressed;
	}ImageButton_t;
	
	
		typedef enum {
		ANIM_PLAY,
		ANIM_PAUSE,
		ANIM_LOADING
	} AnimationState;
	
	typedef enum { 
		PLAYER_STOPPED,
		PLAYER_PLAYING,
		PLAYER_PAUSED
	} PlayerState;
	

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
	#define NEXT_BTN_PIN GPIO_PIN_0
	#define BACK_BTN_PIN GPIO_PIN_1
	#define OPTION_BTN_PIN GPIO_PIN_2
	#define BTN_PORT GPIOB

	
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
	extern const uint16_t next_btn_normal[3200];
	extern const uint16_t next_btn_pressed[3200];
  extern const uint16_t back_btn_normal[3200];
  extern const uint16_t back_btn_pressed[3200];
	static uint8_t anim_frame = 0;
	Page_t currentPage = PAGE_MAIN;
	uint8_t redScreen = 0; //tracks if option botton has been pressed
	
	PlayerState playerState = PLAYER_STOPPED;
	uint8_t animationFrame = 0;
	

	

	

	
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
	
	void DrawPlayIcon(uint16_t x, uint16_t y, uint16_t size, uint16_t color)
	{
		//draw play triangle
		
		for(uint8_t i = 0; i < size/2 ; i++)
		{
			ILI9341_DrawLine(x, y + i, x, y +size - i , color);
			ILI9341_DrawLine(x, y + i, x + size , y +size /2 , color);
			ILI9341_DrawLine(x, y + size - i, x + size, y +size /2 , color);
		}
	}
	
	void DrawPauseIcon(uint16_t x, uint16_t y, uint16_t size, uint16_t color)
	{
		//draw pause barwith animation effect
		uint8_t barWidth = size/2;
		uint8_t barHeight = size;
		uint8_t spacing = size/8;
		
		//left bar(growing animation)
		uint8_t currentHeight = (animationFrame < barHeight) ? animationFrame : barHeight;
		ILI9341_FillRectangle(x, y + (barHeight - currentHeight)/2, barWidth, currentHeight, color);
		
		//right bar(growing animation)
		currentHeight = (animationFrame < barHeight) ? animationFrame : barHeight;
		ILI9341_FillRectangle(x + barWidth + spacing, y + (barHeight - currentHeight)/2, barWidth, currentHeight, color);
		
		
	}
	
	

	
	void DrawPlayerControls()
	{
		uint16_t x = 150, y = 200, size = 30;
		
		//clear previous control area
		
		ILI9341_FillRectangle(x, y, size+50,  size+50, COLOR_BLACK);
		
		switch(playerState)
		{
			case PLAYER_PLAYING:
				DrawPauseIcon(x, y, size, COLOR_RED);
				break;
			case PLAYER_PAUSED:
			case PLAYER_STOPPED:
					DrawPlayIcon(x, y, size, COLOR_GREEN);
					break;
		}
		
		//add animation effects
//		if(animationFrame < 255)
//		{
//			uint8_t pulseSize = animationFrame/10;
//			ILI9341_DrawRectangle(x-pulseSize, y-pulseSize, (size+2)*pulseSize, (size+2)*pulseSize, COLOR_WHITE);
//		}
		
		
	}
	
	
	
	
	
		void DrawPlayPauseAnimation(uint16_t x, uint16_t y, uint16_t size, AnimationState state, uint16_t fg_color, uint16_t bg_color, uint8_t frame)
	{
		ILI9341_FillRectangle(x, y, size, size, bg_color);
		switch(state) {
			case ANIM_PLAY:
				//DRAW PLAY TRIANGLE
			for(uint8_t i=0; i < (size/2) ;i++) {
				ILI9341_DrawLine(x + (size/4), y + i, x + (size/4), y + size - i, fg_color);
				ILI9341_DrawLine(x + (size/4), y + i, x + (3*(size/4)), y + (size/2), fg_color);
				ILI9341_DrawLine(x + (size/4), y + size - i, x + (3*(size/4)), y + (size/2), fg_color);
	}
			break;
	
			case ANIM_PAUSE:
			ILI9341_FillRectangle(x + (size/4),y + (size/4), size/6, size/2, fg_color);
			ILI9341_FillRectangle(x + (3*(size/4)) - (size/6),y + (size/4), size/6, size/2, fg_color);
			break;
		
			case ANIM_LOADING: {
				uint16_t center_x = x + size/2;
				uint16_t center_y = y + size/2;
				uint16_t radius = size/3;
			
					for(uint8_t i=0;i < 8; i++) {
						float angle = (2 * 3.14159 * ((i+frame) % 8)) / 8;
						uint16_t px = center_x + (radius * cos(angle));
						uint16_t py = center_y + (radius * sin(angle));
						ILI9341_FillCircle(px, py, size/8, fg_color);
					}
				
					break;
				}
				}
		
				
			}	
	
	  void HandlePlayerButton() {
		static uint32_t lastPressTime = 0;
		static uint8_t wasPressed = 0;
		
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET)
		{
		if(!wasPressed && (HAL_GetTick() - lastPressTime > 200))
		{
			wasPressed = 1;
			lastPressTime = HAL_GetTick();
			
			//STATE TRANSITION WITH EFFECT
			animationFrame = 0; // reset animation
			
			switch(playerState)
			{
				case PLAYER_STOPPED:
				case PLAYER_PAUSED:
					playerState = PLAYER_PLAYING;
					break;
				case PLAYER_PLAYING:
					playerState = PLAYER_PAUSED;
					break;
			}
		}
	}else 
		{
			wasPressed = 0;
		}
		
		if(animationFrame <255)
		{
			animationFrame +=5;
		}
		
	}		
				
		ImageButton_t next_button = {
		
		.x = 240, .y = 190, .width = 80, .height = 40,
		.normal_img = next_btn_normal, .pressed_img = next_btn_pressed, .is_pressed = 0
		
	};
	
		ImageButton_t back_button = {
		
		.x = 10, .y = 190, .width = 80, .height = 40,
		.normal_img = back_btn_normal, .pressed_img = back_btn_pressed, .is_pressed = 0
		
	};
		
	//botton press check with debounce
	
	uint8_t IsBottonPressed(uint16_t bottonPin)
	{
		static uint32_t lastPressTime = 0;
		if(HAL_GPIO_ReadPin(BTN_PORT, bottonPin) == GPIO_PIN_RESET)
		{
			if(HAL_GetTick() - lastPressTime > 200)
			{
				//200 ms debounce
				lastPressTime = HAL_GetTick();
				return 1;
			}
		}
		
		return 0;
		
	}
	
		void DrawButton(ImageButton_t* btn)
	{
		if(btn->is_pressed)
		{
			ILI9341_DrawImage(btn->x, btn->y, btn->width, btn->height, btn->pressed_img);
		}
		else
		{
			ILI9341_DrawImage(btn->x, btn->y, btn->width, btn->height, btn->normal_img);
		}
	}
	
	//Draw Current Page
	void DrawPage() 
	{
		if(redScreen)
		{
			ILI9341_FillScreen(COLOR_RED);
		}else
		{
			ILI9341_FillScreen(COLOR_BLACK);
		}
		
		if(currentPage == PAGE_MAIN)
		{
			ILI9341_DrawString(100,10,"MAIN PAGE", COLOR_WHITE, COLOR_BLACK, 2);
			
		}
		else{
			ILI9341_DrawString(100,10,"SECOND PAGE", COLOR_WHITE, COLOR_BLACK, 2);
						
		}
		ILI9341_DrawString(10,10,"Ver.00", COLOR_RED, COLOR_BLACK, 1);
		DrawButton(&next_button);
		DrawButton(&back_button);
	}
	
	

	
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	
	  ILI9341_Init(&hspi1, GPIOA, GPIO_PIN_2, GPIOA, GPIO_PIN_1, GPIOA, GPIO_PIN_0);
		ILI9341_FillScreen(COLOR_BLACK);

	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
		
		
	HandlePlayerButton();
	DrawPlayerControls();
	HAL_Delay(20);
		
		
//		if(currentPage == PAGE_MAIN) {
//			DrawPlayPauseAnimation(100,50,60, ANIM_PLAY, COLOR_WHITE, COLOR_BLACK, anim_frame);
//			anim_frame++;
//			HAL_Delay(10);
//		}
//		
//		
		//next botton - go to second page
		if(IsBottonPressed(NEXT_BTN_PIN))
		{
			next_button.is_pressed = 1;
			DrawPage();
			HAL_Delay(100);
			currentPage = PAGE_SECOND;
			next_button.is_pressed = 0;
			DrawPage();
		}

		//back botton - back to main page		
		if(IsBottonPressed(BACK_BTN_PIN))
		{
			back_button.is_pressed = 1;
			DrawPage();
			HAL_Delay(100);
			currentPage = PAGE_MAIN;
			back_button.is_pressed = 0;
			DrawPage();
		}
		
		//option botton - toggle screen color
//	  if(IsBottonPressed(OPTION_BTN_PIN))
//		{
//			redScreen = !redScreen; //toggle state
//			DrawPage();
//		}
		HAL_Delay(10);
  }
  /* USER CODE END 3 */
    }
	

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RESET_Pin|DC_Pin|CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RESET_Pin DC_Pin CS_Pin */
  GPIO_InitStruct.Pin = RESET_Pin|DC_Pin|CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Next_Btn_Pin Back_Btn_Pin Option_Btn_Pin */
  GPIO_InitStruct.Pin = Next_Btn_Pin|Back_Btn_Pin|Option_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

	
	
	
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
