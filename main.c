#include "lcd.h"
#define MCU_CLOCK 1000000
#define PWM_FREQUENCY 50
#define SERVO_STEPS 180 #define SERVO_MIN 700 #define
SERVO_MAX 3000
unsigned int PWM_Period = (MCU_CLOCK / PWM_FREQUENCY);
unsigned int PWM_Duty = 0;
int miliseconds;
int ms;
int sec;
int distance;
long sensor;
int ADC_value=0, ADC_old = 0;
unsigned int servo_stepval, servo_stepnow;
unsigned int servo_lut[ SERVO_STEPS+1 ];
unsigned int i;
int pos = 0;
void ConfigureAdc(void);
void configureservo(void);
void configureus(void);
BCSCTL1 = CALBC1_1MHZ;
 DCOCTL = CALDCO_1MHZ;
 WDTCTL = WDTPW + WDTHOLD;
 P1IFG = BIT0;
 P1DIR |= BIT0;
 P1OUT &= ~BIT0;
 _BIS_SR(GIE);
 configureservo();
 ConfigureAdc();
 configureus();
lcd_init();
while(1){
 ADC10CTL0 |= ENC + ADC10SC;
 ADC_value = ADC10MEM;
 P1IE &= ~0x01;
 P1DIR |= BIT4;
 P1OUT |= BIT4;
 __delay_cycles(10);
 P1OUT &= ~BIT4;
 P1DIR &= ~BIT1;
 P1IFG = 0x00;
 P1IE |= BIT1;
 P1IES &= ~BIT1;
 __delay_cycles(30000);
 distance = sensor/58;
 if((distance * 10) < ADC_value)
 {
 P1OUT |= 0x01;
 TACCR1 = servo_lut[0];
 pos = 1;
 sec=0;
 }
 else
 {
 if ((pos ==1) && (sec > 60))
 {
 P1OUT &= ~0x01;
 TACCR1 = servo_lut[90];
 sec=0;
 ms=0;
 pos=0;
 }
 }
}
}
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
 if(P1IFG&BIT1)
 {
 if(!(P1IES&BIT1))
 {
 TA1CTL|=TACLR;
 miliseconds = 0;
 P1IES |= BIT1;
 }
 else
 {
 sensor = (long)miliseconds*1000 + (long)TA1R;
 }
 P1IFG &= ~BIT1;
 }
}
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
ms+=20;
if (ms==1000)
{
sec+=1;
ms=0;
}
}
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A1 (void)
{
 miliseconds++;
}
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{

 if ((ADC_value/300) != (ADC_old/300))
 {
 LCD_CLR();
 send_string("Limit ");
 send_int(ADC_value/10);
 send_string("cm");
 ADC_old = ADC_value;
 }
 ADC10CTL0 &= ~ADC10IFG;
}
void ConfigureAdc(void)
{
 ADC10CTL1 = INCH_3 + ADC10DIV_3 ;
 ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;
 ADC10AE0 |= BIT3;
 P1SEL |= BIT3;
}
void configureservo(void)
{

 servo_stepval = ( (SERVO_MAX - SERVO_MIN) / SERVO_STEPS );
 servo_stepnow = SERVO_MIN;
 for (i = 0; i < SERVO_STEPS; i++)
 {
 servo_stepnow += servo_stepval;
 servo_lut[i] = servo_stepnow;
 }
 TA0CCTL0 = CCIE;
 TACCTL1 = OUTMOD_7;
 TACTL = TASSEL_2 + MC_1;
 TACCR0 = PWM_Period-1;
 TACCR1 = PWM_Duty;
 P1DIR |= BIT2;
 P1SEL |= BIT2;
}
void configureus(void)
{
 TA1CCTL0 = CCIE;
 TA1CCR0 = 1000;
 TA1CTL = TASSEL_2 + MC_1;
}
