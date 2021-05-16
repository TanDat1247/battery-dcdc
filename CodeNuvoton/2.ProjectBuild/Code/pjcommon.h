#ifndef	_COMMON_H__
#define	_COMMON_H__
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"

#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <math.h>

/* define */
#define DISABLE 0
#define ENABLE 	1

#define	PWM_H_INIT	P12_PushPull_Mode
#define	PWM_H_HIGH	P12 = 1 
#define	PWM_H_LOW		P12 = 0
#define	PWM_H_TOG		P12 ^= 1

#define	PWM_L_INIT	P11_PushPull_Mode
#define	PWM_L_HIGH	P11 = 1 
#define	PWM_L_LOW		P11 = 0
#define	PWM_L_TOG		P11 ^= 1


#define	EN_DRV_INIT	P10_PushPull_Mode
#define	EN_DRV_HIGH	P10 = 1 
#define	EN_DRV_LOW		P10 = 0
#define	EN_DRV_TOG		P10 ^= 1
/* Define for timer 0 and timer 1 */
#define TH0_VALUE        (1333) //1ms, xung clock timer 16/12 Mhz
#define TH1_VALUE        (1333) //1ms, xung clock timer 16/12 Mhz


#define TASK1 10
#define TASK2 10 
#define TASK3 100
#define	TASK4 500
#define	TASK5 500

void SystemInit(void);
void InitClock(void);
void TimerTickInit(void);
static void Timer_Timming_Hw_Init(void);

void GPIO_User_Init(void);
void InitialUART0_Timer3(UINT32 u32Baudrate);

void delay_ms(uint16_t x);
////////////////////////////
void delay_us(uint16_t time);

////////////////////////////
void UartSendBuffer(void);
void  Task1(void);
void  Task2(void);
void  Task3(void);
void	Task4(void);
void 	Task5(void);

extern volatile UINT8 u8TH0_Tmp,u8TL0_Tmp,u8TH1_Tmp,u8TL1_Tmp;
//////////////////////////////
#endif