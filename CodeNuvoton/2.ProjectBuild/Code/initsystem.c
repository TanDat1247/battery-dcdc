#include "pjcommon.h"
//#include "Function_define.h"
bit BIT_TMP;
void SystemInit(void)
{
    InitClock();
    PwmInit();
    GPIO_User_Init();
    TimerTickInit();
    Timer_Timming_Hw_Init();

    /*INIT UART*/
    InitialUART0_Timer3(9600);
    TI = 1;															// Important, use prinft function must set TI=1;
    set_ES;
    /*INIT UART*/
    //PwmInit();	
    set_ADCF;	
}

void InitClock(void)
{
	/* MCU power on system clock is HIRC (16 MHz) */
	/* Mac dinh xung clock la 16Mhz */
}

void TimerTickInit(void)
{
//  - In this example we need to generate a time base equal to 1 ms
    clr_T0M;
    TMOD&=0xF0;TMOD|=0x01;
    u8TH0_Tmp = (65536-TH0_VALUE)/256;
    u8TL0_Tmp = (65536-TH0_VALUE)%256;    
    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;
    set_ET0;                                    //enable Timer0 interrupt 
    set_EA;                                     //enable interrupts
    set_TR0;                                    //Timer0 run   
}
void GPIO_User_Init(void){
//	RELAY_1_INIT;
    EN_DRV_INIT;
	EN_DRV_LOW;
	
	PWM0_P12_OUTPUT_DISABLE;
	PWM1_P11_OUTPUT_DISABLE;
	PWM_H_LOW;
	PWM_L_LOW;
}
static void Timer_Timming_Hw_Init(void){
    clr_T1M;
    //TMOD&=0x0F;TMOD|=0x10;
    TIMER1_MODE1_ENABLE;
    u8TH1_Tmp = (65536-TH1_VALUE)/256;
    u8TL1_Tmp = (65536-TH1_VALUE)%256;  
    TH1 = u8TH1_Tmp;
    TL1 = u8TL1_Tmp;
    set_ET1;                                    //enable Timer1 interrupt
    set_EA;                                     //enable interrupts
    set_TR1;                                    //Timer1 run
}

#if 0
char putchar (char c)
{
    SBUF_1 = c;      /* output character */
    while (!TI_1);  /* wait until transmitter ready */
    TI_1 = 0;
    return (c);
}
#else
char putchar (char c)  {
    SBUF = c;
    while (!TI);
    TI = 0;
    return (c);
}
#endif

void InitialUART0_Timer3(UINT32 u32Baudrate) //use timer3 as Baudrate generator
{
    P06_Quasi_Mode;		//Setting UART pin as Quasi mode for transmit
    P07_Quasi_Mode;		//Setting UART pin as Quasi mode for transmit	
	
    SCON = 0x50;     //UART0 Mode1,REN=1,TI=1
    set_SMOD;        //UART0 Double Rate Enable
    T3CON &= 0xF8;   //T3PS2=0,T3PS1=0,T3PS0=0(Prescale=1)
    set_BRCK;        //UART0 baud rate clock source = Timer3

    #ifdef FOSC_160000
        RH3    = HIBYTE(65536 - (1000000/u32Baudrate)-1);  		/*16 MHz */
        RL3    = LOBYTE(65536 - (1000000/u32Baudrate)-1);			/*16 MHz */
    #endif
    #ifdef FOSC_166000
        RH3    = HIBYTE(65536 - (1037500/u32Baudrate)); 			/*16.6 MHz */
        RL3    = LOBYTE(65536 - (1037500/u32Baudrate)); 			/*16.6 MHz */
    #endif
    set_TR3;         //Trigger Timer3
	//set_TI;					 //For printf function must setting TI = 1
}

void PwmInit(void)
{
    Set_All_GPIO_Quasi_Mode;
    PWM_H_INIT;
    PWM_L_INIT;
/*  PWM frequency = Fpwm/((PWMPH,PWMPL) + 1) <Fpwm = Fsys/PWM_CLOCK_DIV>    */
/*-----------------------------------------------------------------------------------------------------------------
	PWM frequency 	= Fpwm/((PWMPH,PWMPL)+1) = (16MHz/2)/(0x7CF+1) = 1KHz (1ms)
	PWM4 high duty 	= PWM4H,PWM4L = 0x01CF = 1/4 PWM period
	PWM0 high duty 	= PWM0H,PMW0L = 0x03CF = 1/2 PWM period
	Dead time 			= 0x1FF <PDTEN.4+PDTCNT[7:0]>/Fsys = 0x1FF/Fsys = 512/16000000 = 32 us (max value)
-----------------------------------------------------------------------------------------------------------------*/
    PWM_CLOCK_FSYS;
    PWM_CLOCK_DIV_2;
    PWM_COMPLEMENTARY_MODE;					//Only this 2mode support deat time function
    PWMPH = 0x00;
    PWMPL = 0xC7;            

    PWM0H = 0x00;
    PWM0L = 0x00;
    //set_SFRPAGE;										// PWM4 and PWM5 duty value need set SFPPAGE 1
//		PWM4H = 0x01;
//		PWM4L = 0xCF;
    //clr_SFRPAGE;
    
    PWM01_DEADTIME_ENABLE;
    //PWM45_DEADTIME_ENABLE;
    PWM_DEAD_TIME_VALUE(0x00F);			//value never over 0x1FF
		
//Please always setting Dead time if needed before PWM run.		
    set_LOAD;
    set_PWMRUN;
}

void PWM_DEAD_TIME_VALUE(UINT16	DeadTimeData)
{
	UINT8 deadtmphigh,deadtmplow;
	deadtmplow = DeadTimeData;
	deadtmphigh = DeadTimeData>>8;
	BIT_TMP = EA;
	if (deadtmphigh==0x01)
	{
		EA = 0;
		TA = 0xAA;
		TA = 0x55;
		PDTEN|=0x10;
	}
	TA = 0xAA;
	TA = 0x55;
	PDTCNT = deadtmplow;
	EA = BIT_TMP;
}