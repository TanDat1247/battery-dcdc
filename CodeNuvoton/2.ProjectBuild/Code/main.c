#include "pjcommon.h"
#include "Function_define.h"
#include "string.h"

volatile uint16_t sys_millis = 0;
#define UART_BUFFER_MAX_CNT 100
xdata char cUartBuffer[UART_BUFFER_MAX_CNT];

////////////////////
#define STATE_0 0
#define STATE_1 1
#define STATE_2 2
volatile uint8_t MainState = STATE_0;

#define MAX_DUTY_BOOST 180   // Vin PWM min = 14.4 (13V=90%)
#define MIN_DUTY_BOOST 154   // Vin max = 16.8V    (13V=77%)

#define MAX_CYCLE 200
#define VIN_OFFSET_ADC 1474		//13V
#define VIN_THRESHOLD  1631     //14.4V

void PwmSet(uint8_t Duty, uint8_t Enable);
uint8_t Duty_Ctr = MAX_DUTY_BOOST;
uint8_t OutputEnable = 0;

uint16_t VoltageIn = 0;
uint16_t VoltageOut = 0;

typedef struct {
    uint8_t Task1;
    uint8_t Task2;
    uint8_t Task3;
    uint8_t Task4;
    uint8_t Task5;
}_StrTask;
_StrTask StrTask;

volatile bit bFlagDelay = 0;
volatile uint32_t uCountDelay = 0;
uint16_t u16Cnt = 0;
/* Main loop */


void main(void) {
    Set_All_GPIO_Quasi_Mode;
    SystemInit();
    delay_ms(100);
    EN_DRV_LOW;
  //clr_LOAD;
  //clr_CLRPWM;
  //	PwmSet(10,BOOST,0);

    delay_ms(2000);
  //PWM0L = 50;
    while (1) {
		
        switch (MainState) {
            case STATE_0:{
                while (1) {
                Task1();
                Task4();
                //Task5();
                if (MainState != STATE_0) {break;}
              }
              break;
            }
            case STATE_1:{
                while (1) {
                Task2();
                Task4();
                Task5();
                if (MainState != STATE_0) {break;}
              }
              break;
            }
            case STATE_2:{
                while (1) {
                Task3();
                Task4();
                Task5();
                if (MainState != STATE_0) {break;}
              }
              break;
            }
        }
    }
}


/* Variable TIMER */
volatile UINT8 u8TH0_Tmp, u8TL0_Tmp, u8TH1_Tmp, u8TL1_Tmp;
/************************************************************************************************************
 *		TIMER 0 interrupt subroutine
 ************************************************************************************************************/
void Timer0_ISR(void) interrupt 1 //interrupt address is 0x000B
{
    /* Reload counter */
    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;
    /* Increase system click and set flag task enable */
    sys_millis++;
    if (bFlagDelay == 1) {uCountDelay++;} 
	else {uCountDelay = 0;}
    if (sys_millis >= 10000) sys_millis = 0;
    if (sys_millis % TASK1 == 0) StrTask.Task1 = 1;
    if (sys_millis % TASK2 == 0) StrTask.Task2 = 1;
    if (sys_millis % TASK3 == 0) StrTask.Task3 = 1;
    if (sys_millis % TASK4 == 0) StrTask.Task4 = 1;
    if (sys_millis % TASK5 == 0) StrTask.Task5 = 1;
}
/************************************************************************************************************
 *		TIMER 1 interrupt subroutine
 ************************************************************************************************************/
#define	 NUM_OF_CNT_ADC	100
uint8_t u8AdcCnt = 0;

uint16_t NumOfCntAdc_6 = 0;
uint32_t TotalOfAdc_6 = 0;

uint16_t NumOfCntAdc_5 = 0;
uint32_t TotalOfAdc_5 = 0;

void Timer1_ISR(void) interrupt 3
{
    TH1 = u8TH1_Tmp;
    TL1 = u8TL1_Tmp;
	if(ADCF == 1){
		switch (u8AdcCnt) {
			case 0:{
				TotalOfAdc_5 += ADCRH * 16 + ADCRL;
				NumOfCntAdc_5++;
				if(NumOfCntAdc_5>=NUM_OF_CNT_ADC){
					VoltageOut = (uint16_t)(TotalOfAdc_5/NUM_OF_CNT_ADC);
					TotalOfAdc_5 = 0;
					NumOfCntAdc_5 = 0;
				}
				Disable_ADC;
				Enable_ADC_AIN6;
				clr_ADCF;
				set_ADCS;
				u8AdcCnt = 1;
				break;
			}
			case 1:{
				TotalOfAdc_6 += ADCRH * 16 + ADCRL;
				NumOfCntAdc_6++;
				if(NumOfCntAdc_6>=NUM_OF_CNT_ADC){
					VoltageIn = (uint16_t)(TotalOfAdc_6/NUM_OF_CNT_ADC);
					TotalOfAdc_6 = 0;
					NumOfCntAdc_6 = 0;
				}
				Disable_ADC;
				Enable_ADC_AIN5;
				clr_ADCF;
				set_ADCS;
				u8AdcCnt = 0;
				break;
			}
		}
	}
}

volatile uint8_t DRV_State = 0;
void Task1(void) {
  if (StrTask.Task1 == 1) {
    if ((OutputEnable == 0)&&(DRV_State!=1)) {
		PwmSet(5, 0);
		DRV_State = 1;
    } 
    else if (OutputEnable == 1) {
		if((DRV_State!=2)&&(VoltageIn<VIN_THRESHOLD))        //100% Duty Cycle - Vin<14.4
		{
			EN_DRV_LOW;
			PWM0_P12_OUTPUT_DISABLE;
			PWM1_P11_OUTPUT_DISABLE;
			
			PWM_L_LOW;
			PWM_H_HIGH;
			EN_DRV_HIGH;
			DRV_State = 2;
			
			Duty_Ctr = MAX_DUTY_BOOST;
		}
		else if(VoltageIn>=VIN_THRESHOLD)      //Vin > 14.4
		{
			unsigned int duty_tmp;
			duty_tmp = 1.0*MAX_CYCLE*VIN_OFFSET_ADC/VoltageIn;    //1446 => 13V ADC OFFSET
			
			if(duty_tmp<MIN_DUTY_BOOST){
				duty_tmp =  MIN_DUTY_BOOST;
			}
			
			if(Duty_Ctr!=duty_tmp){
				if(Duty_Ctr>duty_tmp) Duty_Ctr--;
				else if(Duty_Ctr<duty_tmp)  Duty_Ctr++;
				
				PwmSet(Duty_Ctr, 1);
				DRV_State = 3;
			}
			else if(DRV_State!=3){
				PwmSet(Duty_Ctr, 1);
				DRV_State = 3;
			}
		}
    }
    StrTask.Task1 = 0;
  }
}

void Task2(void) {
  if (StrTask.Task2 == 1) {

    StrTask.Task2 = 0;
  }
}

void Task3(void) {
  if (StrTask.Task3 == 1) {

    StrTask.Task3 = 0;
  }
}

void Task4(void) {
	uint8_t i =0;
    if (StrTask.Task4 == 1) {
        if(strstr(cUartBuffer,"ON")!=NULL){
            OutputEnable=1;
            for(i=0;i<UART_BUFFER_MAX_CNT;i++){cUartBuffer[i]=0;}
            u16Cnt=0;
        }
        else if(strstr(cUartBuffer,"OFF")!=NULL){
            OutputEnable=0;
            for(i=0;i<UART_BUFFER_MAX_CNT;i++){cUartBuffer[i]=0;}
            u16Cnt=0;
        }
        StrTask.Task4 = 0;
    }
}
void Task5(void) {
  if (StrTask.Task5 == 1) {

    StrTask.Task5 = 0;
  }
}

void UartSendBuffer(void) {

}

void delay_us(uint16_t time) {
  uint16_t i;
  for (i = 0; i < time; i++) {
    ;
  }
}

void delay_ms(uint16_t x) {
  static uint32_t uDelay = 0;
  uDelay = x;
  bFlagDelay = 1;
  uCountDelay = 0;
  while (uCountDelay <= uDelay) {}
  bFlagDelay = 0;
}


void SerialPort0_ISR(void) interrupt 4 {
  if (RI == 1) {
    if (u16Cnt < UART_BUFFER_MAX_CNT) {
      cUartBuffer[u16Cnt] = SBUF;
      u16Cnt++;
    } else {
      u16Cnt = 0;
    }
    clr_RI;
  }

}

void WakeUp_Timer_ISR(void) interrupt 17 //ISR for self wake-up timer
{
  //set_SWRST;
  clr_WKTF; //clear interrupt flag   	
}
/////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_FULL_ASSERT

/**
 * @brief	Reports the name of the source file and the source line number
 *	 where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval : None
 */
void assert_failed(u8 * file, u32 line) {
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1) {}
}
#endif

void PwmSet(uint8_t Duty, uint8_t Enable) {
    if (Enable == 0) {
        EN_DRV_LOW;
        PWM0_P12_OUTPUT_DISABLE;
        PWM1_P11_OUTPUT_DISABLE;
        PWM_H_LOW;
        PWM_L_LOW;
    } 
    else if (Enable == 1) {
        PWM0_P12_OUTPUT_ENABLE;
        PWM1_P11_OUTPUT_ENABLE;
        EN_DRV_HIGH;
        
		PWM0H = HIBYTE(Duty);    
		PWM0L = LOBYTE(Duty); //High driver
		set_LOAD;
    }
}