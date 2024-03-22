/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "math.h"
#include "flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PI 3.14159265
#define MAX_LED 350
#define MAX_LED1 300
#define MAX_LED2 50
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

TIM_HandleTypeDef htim1;
DMA_HandleTypeDef hdma_tim1_ch1;
DMA_HandleTypeDef hdma_tim1_ch2;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_CAN_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t LED_Data[MAX_LED][4];
uint16_t pwmData1[(24*MAX_LED1)+40]; 			// front_left, front_right, right, back_right, back, back_left, left
uint16_t pwmData2[(24*MAX_LED2)+40];			// flag
uint8_t numLEDs[8];
uint8_t data[8];
volatile uint8_t datasentflag1=0, datasentflag2=0;
uint16_t effStep[8] = {0,0,0,0,0,0,0};
uint8_t color_Manual[3]={3,221,255}, color_Auto[3]={0,255,0}, color_Turn[3]={255,192,0},color1[3] ={255,165,0}, color_Error[3]={255,0,0}, color_Off[3]={0,0,0}, color_White[3]={255,255,255};
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
  if (htim->Instance == TIM1)
  {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
      datasentflag1= 1;
    }
		/*if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_2);
      datasentflag2= 1;
    }*/
	}
}

void Set_LED(int LEDnum, int Red, int Green, int Blue){
	LED_Data[LEDnum][0] = LEDnum;
	LED_Data[LEDnum][1] = Green;
	LED_Data[LEDnum][2] = Red;
	LED_Data[LEDnum][3] = Blue;
}
void WS2812_Send(){
	uint32_t indx1=0, indx2=0;
	uint32_t color;
	uint16_t num_led =0;
	for (int i = 0; i < 7; i++) num_led += numLEDs[i];
	for (int i = 0; i<MAX_LED1; i++)
	//for (int i = 0; i<num_led; i++)
	{
		color = ((LED_Data[i][1]<<16) | (LED_Data[i][2]<<8) | (LED_Data[i][3]));
		for (int j=0; j<24; j++)
		{
			if ((color >> (23 - j)) & 1)
			{
				pwmData1[indx1] = 60;  // 2/3 of 90
			}
			else pwmData1[indx1] = 30;  // 1/3 of 90
			indx1++;
		}
	}
	/*for (int i = 0;i < numLEDs[7]; i++){
		color = ((LED_Data[i+num_led][1]<<16) | (LED_Data[i+num_led][2]<<8) | (LED_Data[i+num_led][3]));
		for (int j=0; j<24; j++)
		{
			if ((color >> (23 - j)) & 1)
			{
				pwmData2[indx2] = 60;  // 2/3 of 90
			}
			else pwmData2[indx2] = 30;  // 1/3 of 90
			indx2++;
		}
	}*/
	for (int i=0; i<40; i++)
	{
		pwmData1[indx1] = 0;
		//pwmData2[indx2] = 0;
		indx1++;
		//indx2++;
	}
	
	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)pwmData1, indx1);
	//HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_2, (uint32_t *)pwmData2, indx2);
	while (!(datasentflag1 /*& datasentflag2*/)){};
	datasentflag1 = 0;
	//datasentflag2 = 0;
}

void Solid(uint8_t strip, uint8_t color[3]){
	uint16_t i = 0;
	for (uint8_t j = 0 ; j <strip; j++) i += numLEDs[j];
	for (uint16_t k = i; k < (i + numLEDs[strip]); k++) Set_LED(k,color[0],color[1],color[2]);
}
uint8_t Rainbow(uint8_t strip) {
    // Strip ID: 0 - Effect: Rainbow - LEDS: numLEDs[strip]
    // Steps: numLEDs[strip] - Delay: 20
    // Colors: 3 (255.0.0, 0.255.0, 0.0.255)
    // Options: rainbowlen=numLEDs[strip], toLeft=true,

    float factor1, factor2;
    uint16_t ind;
		uint16_t i = 0;
		for (uint8_t j = 0 ; j <strip; j++) i += numLEDs[j];
    for (uint16_t j = i; j < (i + numLEDs[strip]); j++) {
        ind = effStep[strip] + j * 1;
        switch ((int)((ind % numLEDs[strip]) / (numLEDs[strip] / 3))) {
        case 0:
            factor1 = 1.0 - ((float)(ind % numLEDs[strip] - 0 * (numLEDs[strip] / 3)) / (numLEDs[strip] / 3));
            factor2 = (float)((int)(ind - 0) % numLEDs[strip]) / (numLEDs[strip] / 3);
            Set_LED(j, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2);
            break;
        case 1:
            factor1 = 1.0 - ((float)(ind % numLEDs[strip] - 1 * (numLEDs[strip] / 3)) / (numLEDs[strip] / 3));
            factor2 = (float)((int)(ind - (numLEDs[strip] / 3)) % numLEDs[strip]) / (numLEDs[strip] / 3);
            Set_LED(j, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2);
            break;
        case 2:
            factor1 = 1.0 - ((float)(ind % numLEDs[strip] - 2 * (numLEDs[strip] / 3)) / (numLEDs[strip] / 3));
            factor2 = (float)((int)(ind - 2 * (numLEDs[strip] / 3)) % numLEDs[strip]) / (numLEDs[strip] / 3);
            Set_LED(j, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2);
            break;
        }
    }
    if (effStep[strip] >= numLEDs[strip]) {
        effStep[strip] = 0;
        return 0x03;
    } else
        effStep[strip]++;
    return 0x01;
}
uint8_t Pulse(uint8_t strip, uint8_t step, uint8_t color[3]) {
	uint16_t i = 0;
	for (uint8_t j = 0 ; j <strip; j++) i += numLEDs[j];
	for (uint16_t j = i; j < (i + numLEDs[strip]); j++) 	Set_LED(j,(uint8_t) color[0]*tan(effStep[strip]*PI/4000), (uint8_t)color[1]*tan(effStep[strip]*PI/4000), (uint8_t)color[2]*tan(effStep[strip]*PI/4000));
	if(effStep[strip]  >= 1000) {effStep[strip] = 0; return 0x03; }
	else effStep[strip] += 5;
  return 0x01;
}
uint8_t Blink(uint8_t strip, uint16_t timOn, uint16_t timOff, uint8_t color1[3], uint8_t color2[3]) {
	uint16_t i = 0;
	for (uint8_t j = 0 ; j <strip; j++) i += numLEDs[j];
  if(effStep[strip] < (timOn*8/100)) {
    for (uint16_t j = i; j < (i + numLEDs[strip]); j++) 
      Set_LED(j, color1[0], color1[1], color1[2]);
  }																															//1000ms corresponds to about 80 steps
  else {
    for (uint16_t j = i; j < (i + numLEDs[strip]); j++) 
      Set_LED(j, color2[0], color2[1], color2[2]);
  }
  if(effStep[strip] >= (timOn+timOff)*12/100) {effStep[strip] = 0; return 0x03; }
  else effStep[strip]++;
  return 0x01;
}
void Wheel(uint16_t num, uint8_t WheelPos){
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    Set_LED(num,255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    Set_LED(num,0, WheelPos * 3, 255 - WheelPos * 3);
  }
  else
  {
    WheelPos -= 170;
    Set_LED(num,WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
uint8_t colorCycle(uint8_t strip){
	uint16_t i = 0;
	for (uint8_t j = 0 ; j <strip; j++) i += numLEDs[j];
  for (uint16_t j = i; j < (i + numLEDs[strip]); j++){
		Wheel(j,numLEDs[strip]+effStep[strip]);
  }
	if(effStep[strip] >= 256) {effStep[strip] = 0; return 0x03; }
  else effStep[strip]++;
  return 0x01;
}

uint8_t Turn_Right(uint8_t strip, uint8_t color[3]){
  uint16_t i = 0;
	for (uint8_t j = 0 ; j <strip; j++) i += numLEDs[j];
  for (uint16_t j = i; j < (i + numLEDs[strip]); j++)
  {
		if (j-i < effStep[strip])	Set_LED(j,0,0,0);
		else	Set_LED(j,color[0],color[1],color[2]);
  }
	if(effStep[strip] > numLEDs[strip]) {effStep[strip] = 0; return 0x03; }
  else effStep[strip]+=1;
  return 0x01;
}
uint8_t Turn_Left(uint8_t strip, uint8_t color[3]){
  uint16_t i = 0;
	for (uint8_t j = 0 ; j <strip; j++) i += numLEDs[j];
  for (uint16_t j = (i+numLEDs[strip]-1); j >=i; j--)
  {
		if (i+numLEDs[strip]-j-1 < effStep[strip])	Set_LED(j,0,0,0);
		else	Set_LED(j,color[0],color[1],color[2]);
  }
	if(effStep[strip] >= numLEDs[strip]) {effStep[strip] = 0; return 0x03; }
  else effStep[strip]+=1;
  return 0x01;
}
uint8_t Comet(uint8_t strip, uint8_t len, uint8_t color[3]){
	uint16_t i = 0;
	for (uint8_t j = 0 ; j <strip; j++) i += numLEDs[j];
	for (uint16_t j = i; j < (i + numLEDs[strip]); j++){
		if ((j-i)>=  (effStep[strip]-len) && (j-i)<(effStep[strip])) Set_LED(j,color[0],color[1],color[2]);
    else if ((j-i)>= (effStep[strip]-len-15) && (j-i)<(effStep[strip]-len)) Set_LED(j,(uint8_t) color[0]*(j-i-effStep[strip]+len+15)/28, (uint8_t)color[1]*(j-i-effStep[strip]+len+15)/28, (uint8_t)color[2]*(j-i-effStep[strip]+len+15)/28);
		else Set_LED(j,0,0,0);
	}
	if(effStep[strip]  > numLEDs[strip]) {effStep[strip] = 0; return 0x03;}
	else effStep[strip]++;
  return 0x01;
}
void animation(uint16_t mode){
  if((mode >> 8) == 0){
    if (((mode & 0x000C)>>2)==0){ //Off
      for(uint8_t i=0;i<8;i++) Solid(i,color_Off);
    }
    else if (((mode & 0x000C)>>2)==1){  //Auto
      if((mode & 0x0003)==0){   //Normal
        Solid(0,color_White); Solid(1,color_White); Solid(3,color_White); Solid(5,color_White);
				Solid(2,color_Auto); Solid(4,color_Auto); Solid(6,color_Auto); Solid(7,color_Auto);
      }
      else if((mode & 0x0003)==1){   //Left
        Blink(0,300,300,color_Turn,color_Off); Solid(1,color_White); Solid(2,color_Auto); Solid(3,color_White); 
				Turn_Left(4,color_Auto); Blink(5,300,300,color_Turn,color_Off); Blink(6,300,300,color_Turn,color_Off); Solid(7,color_Auto);
      }
      else if((mode & 0x0003)==2){   //Right
        Solid(0,color_White); Blink(1,300,300,color_Turn,color_Off); Blink(2,300,300,color_Turn,color_Off); Blink(3,300,300,color_Turn,color_Off); 
				Turn_Right(4,color_Auto); Solid(5,color_White); Solid(6,color_Auto); Solid(7,color_Auto);
      }
      else if((mode & 0x0003)==3){   //Error
        for(uint8_t i=0;i<8;i++) Blink(i,300,300,color_Error,color_Auto);
      }
    }
    else if (((mode & 0x000C)>>2)==2){  //Manual
      if((mode & 0x0003)==0){   //Normal
        Solid(0,color_White); Solid(1,color_White); Solid(3,color_White); Solid(5,color_White);
				Solid(2,color_Manual); Solid(4,color_Manual); Solid(6,color_Manual); Solid(7,color_Manual);
      }
      else if((mode & 0x0003)==1){   //Left
        Blink(0,300,300,color_Turn,color_Off); Solid(1,color_White); Solid(2,color_Manual); Solid(3,color_White); Turn_Left(4,color_Manual);
				Blink(5,300,300,color_Turn,color_Off); Blink(6,300,300,color_Turn,color_Off); Solid(7,color_Manual);
      }
      else if((mode & 0x0003)==2){   //Right
        Solid(0,color_White); Blink(1,300,300,color_Turn,color_Off); Blink(2,300,300,color_Turn,color_Off); Blink(3,300,300,color_Turn,color_Off); 
				Turn_Right(4,color_Manual); Solid(5,color_White); Solid(6,color_Manual); Solid(7,color_Manual);
      }
      else if((mode & 0x0003)==3){   //Error
        for(uint8_t i=0;i<8;i++) Blink(i,300,300,color_Error,color_Manual);
      }
    }
    else if (((mode&0x000C)>>2)==3){  //Solid
      Solid(4,color_White);  Solid(7,color_White);
      if((mode & 0x0003)==0){   //Normal
        Solid(0,color_White); Solid(1,color_White); Solid(2,color_White); Solid(3,color_White);
        Solid(5,color_White); Solid(6,color_White);
      }
      else if((mode & 0x0003)==1){   //Left
        Blink(0,300,300,color_Turn,color_Off); Solid(1,color_White); Solid(2,color_White); Solid(3,color_White); 
        Blink(5,300,300,color_Turn,color_Off); Blink(6,300,300,color_Turn,color_Off);
      }
      else if((mode & 0x0003)==2){   //Right
        Solid(0,color_White); Blink(1,300,300,color_Turn,color_Off); Blink(2,300,300,color_Turn,color_Off); Blink(3,300,300,color_Turn,color_Off);
        Solid(5,color_White); Solid(6,color_White);
      }
      else if((mode & 0x0003)==3){   //Error
        Blink(0,300,300,color_Error,color_Off); Blink(1,300,300,color_Error,color_Off); Solid(2,color_White);
        Blink(3,300,300,color_Error,color_Off); Blink(5,300,300,color_Error,color_Off); Solid(6,color_White);
      }
    }      
  }
  else if((mode >> 8)==1 && ((mode & 0x000C)>>2)==3){  //Custom1: Pulse
    Pulse(4,1,color_White); Pulse(7,1,color_White);
      if((mode & 0x0003)==0){   //Normal
        Solid(0,color_White); Solid(1,color_White); Pulse(2,1,color_White); Solid(3,color_White);
        Solid(5,color_White); Pulse(6,1,color_White);
      }
      else if((mode & 0x0003)==1){   //Left
        Blink(0,300,300,color_Turn,color_Off); Solid(1,color_White); Pulse(2,1,color_White); Solid(3,color_White); 
        Blink(5,300,300,color_Turn,color_Off); Blink(6,300,300,color_Turn,color_Off);
      }
      else if((mode & 0x0003)==2){   //Right
        Solid(0,color_White); Blink(1,300,300,color_Turn,color_Off); Blink(2,300,300,color_Turn,color_Off); Blink(3,300,300,color_Turn,color_Off);
        Solid(5,color_White); Pulse(6,1,color_White);
      }
      else if((mode & 0x0003)==3){   //Error
        Blink(0,300,300,color_Error,color_Off); Blink(1,300,300,color_Error,color_Off); Pulse(2,1,color_White);
        Blink(3,300,300,color_Error,color_Off); Blink(5,300,300,color_Error,color_Off); Pulse(6,1,color_White);      
      }
  }
  else if((mode >> 8) == 2 && ((mode & 0x000C)>>2)==3){  //Custom2: Rainbow
    Rainbow(4); Rainbow(7);
      if((mode & 0x0003)==0){   //Normal
        Solid(0,color_White); Solid(1,color_White); Rainbow(2); Solid(3,color_White);
        Solid(5,color_White); Rainbow(6);
      }
      else if((mode & 0x0003)==1){   //Left
        Blink(0,300,300,color_Turn,color_Off); Solid(1,color_White); Rainbow(2); Solid(3,color_White); 
        Blink(5,300,300,color_Turn,color_Off); Blink(6,300,300,color_Turn,color_Off);
      }
      else if((mode & 0x0003)==2){   //Right
        Solid(0,color_White); Blink(1,300,300,color_Turn,color_Off); Blink(2,300,300,color_Turn,color_Off); Blink(3,300,300,color_Turn,color_Off);
        Solid(5,color_White); Rainbow(6);
      }
      else if((mode & 0x0003)==3){   //Error
        Blink(0,300,300,color_Error,color_Off); Blink(1,300,300,color_Error,color_Off); Rainbow(2);
        Blink(3,300,300,color_Error,color_Off); Blink(5,300,300,color_Error,color_Off); Rainbow(6);      
      }
  }
  else if((mode >> 8) == 3 && ((mode & 0x000C)>>2)==3){  //Custom3: ColorCycle
    colorCycle(4);colorCycle(7);
      if((mode & 0x0003)==0){   //Normal
        Solid(0,color_White); Solid(1,color_White); colorCycle(2); Solid(3,color_White);
        Solid(5,color_White); colorCycle(6);
      }
      else if((mode & 0x0003)==1){   //Left
        Blink(0,300,300,color_Turn,color_Off); Solid(1,color_White); colorCycle(2); Solid(3,color_White); 
        Blink(5,300,300,color_Turn,color_Off); Blink(6,300,300,color_Turn,color_Off);
      }
      else if((mode & 0x0003)==2){   //Right
        Solid(0,color_White); Blink(1,300,300,color_Turn,color_Off); Blink(2,300,300,color_Turn,color_Off); Blink(3,300,300,color_Turn,color_Off);
        Solid(5,color_White); colorCycle(6);
      }
      else if((mode & 0x0003)==3){   //Error
        Blink(0,300,300,color_Error,color_Off); Blink(1,300,300,color_Error,color_Off); colorCycle(2);
        Blink(3,300,300,color_Error,color_Off); Blink(5,300,300,color_Error,color_Off); colorCycle(6);   
      }
  }
  else if((mode >> 8) == 4 && ((mode & 0x000C)>>2)==3){  //Custom4: Blink
    Blink(4,300,300,color_White,color_Off); Blink(7,300,300,color_White,color_Off);
		if((mode & 0x0003)==0){   //Normal
        Solid(0,color_White); Solid(1,color_White); Blink(2,300,300,color_White,color_Off); Solid(3,color_White);
        Solid(5,color_White); Blink(6,300,300,color_White,color_Off);
      }
      else if((mode & 0x0003)==1){   //Left
        Blink(0,300,300,color_Turn,color_Off); Solid(1,color_White); Blink(2,300,300,color_White,color_Off); Solid(3,color_White); 
        Blink(5,300,300,color_Turn,color_Off); Blink(6,300,300,color_Turn,color_Off);
      }
      else if((mode & 0x0003)==2){   //Right
        Solid(0,color_White); Blink(1,300,300,color_Turn,color_Off); Blink(2,300,300,color_Turn,color_Off); Blink(3,300,300,color_Turn,color_Off);
        Solid(5,color_White); Blink(6,300,300,color_White,color_Off);
      }
      else if((mode & 0x0003)==3){   //Error
        Blink(0,300,300,color_Error,color_Off); Blink(1,300,300,color_Error,color_Off); Blink(2,300,300,color_White,color_Off);
        Blink(3,300,300,color_Error,color_Off); Blink(5,300,300,color_Error,color_Off); Blink(6,300,300,color_White,color_Off);
      }
  }
	else if((mode >> 8) == 5 && ((mode & 0x000C)>>2)==3){  //Custom5: Comet
    Comet(4,1,color_White); Comet(7,1,color_White);
		if((mode & 0x0003)==0){   //Normal
        Solid(0,color_White); Solid(1,color_White); Comet(2,1,color_White); Solid(3,color_White);
        Solid(5,color_White); Comet(6,1,color_White);
      }
      else if((mode & 0x0003)==1){   //Left
        Blink(0,300,300,color_Turn,color_Off); Solid(1,color_White); Comet(2,1,color_White); Solid(3,color_White); 
        Blink(5,300,300,color_Turn,color_Off); Blink(6,300,300,color_Turn,color_Off);
      }
      else if((mode & 0x0003)==2){   //Right
        Solid(0,color_White); Blink(1,300,300,color_Turn,color_Off); Blink(2,300,300,color_Turn,color_Off); Blink(3,300,300,color_Turn,color_Off);
        Solid(5,color_White); Comet(6,1,color_White);
      }
      else if((mode & 0x0003)==3){   //Error
        Blink(0,300,300,color_Error,color_Off); Blink(1,300,300,color_Error,color_Off); Comet(2,1,color_White);
        Blink(3,300,300,color_Error,color_Off); Blink(5,300,300,color_Error,color_Off); Comet(6,1,color_White);
      }
  }
	else for(uint8_t i=0;i<8;i++) Solid(i,color_Off);
}
CAN_TxHeaderTypeDef TxHeader1, TxHeader2;
CAN_RxHeaderTypeDef RxHeader;

uint8_t TxData1[8];
uint8_t TxData2[8];
uint8_t RxData[8];

uint32_t TxMailbox1, TxMailbox2;
uint16_t mode=0, pre_mode=0;
int datacheck = 0;
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan){
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData);
	if (RxHeader.DLC == 2 && RxHeader.ExtId == 0x0000012A){
		for(uint8_t i=0;i<2;i++) data[i]=RxData[i];
		datacheck = 1;
	}
	else if (RxHeader.DLC == 8 && RxHeader.ExtId == 0x0000022A)	datacheck = 2;
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
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
	HAL_CAN_Start(&hcan);

  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
	TxHeader1.DLC = 2;
  TxHeader1.IDE = CAN_ID_EXT;
  TxHeader1.RTR = CAN_RTR_DATA;
  TxHeader1.ExtId = 0x00102A;
	
	TxHeader2.DLC = 8;
  TxHeader2.IDE = CAN_ID_EXT;
  TxHeader2.RTR = CAN_RTR_DATA;
  TxHeader2.ExtId = 0x00112A;
	
	Flash_Read_String(numLEDs,_PAGE_127_,8);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if (datacheck == 1){
			mode = (RxData[0] << 8) | RxData[1];
			if (mode != pre_mode) {
				for(uint8_t i=0;i<8;i++) effStep[i]=0; 
				pre_mode = mode;
			}
			datacheck = 0;
		}
		else if (datacheck == 2){
			//for(uint8_t i=0;i<8;i++) numLEDs[i]=RxData[i];
			Flash_Write_String(RxData,_PAGE_127_,8);
			Flash_Read_String(numLEDs,_PAGE_127_,8);
			datacheck = 0;
		}
		animation(mode);
		WS2812_Send();
		for(uint8_t i=0;i<2;i++) TxData1[i]=data[i];
		for(uint8_t i=0;i<8;i++) TxData2[i]=numLEDs[i];
		HAL_CAN_AddTxMessage(&hcan, &TxHeader1, TxData1, &TxMailbox1);
		HAL_CAN_AddTxMessage(&hcan, &TxHeader2, TxData2, &TxMailbox2);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 18;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */
	CAN_FilterTypeDef canfilterconfig;

  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 10;  // which filter bank to use from the assigned ones
  canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
  canfilterconfig.FilterIdHigh = 0x0000;
  canfilterconfig.FilterIdLow = 0x0954;					// ID: 0x0000012A   (ID+IDE+RTR+0)
  canfilterconfig.FilterMaskIdHigh = 0xFFFF;
  canfilterconfig.FilterMaskIdLow = 0xE7FF;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
  canfilterconfig.SlaveStartFilterBank = 14;  // doesn't matter in single can controllers

  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);
  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 89;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

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
