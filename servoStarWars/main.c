#include "msp430g2553.h" // make sure you change the header to suit your particular device.
// Connect the servo SIGNAL wire to P1.2 through a 1K resistor.

#define MCU_CLOCK			1000000
#define PWM_FREQUENCY		46		// In Hertz, ideally 50Hz.

#define     LED1                  BIT0
#define     LED2                  BIT6
#define     LED_DIR               P1DIR
#define     LED_OUT               P1OUT

#define     BUTTON                BIT3
#define     BUTTON_OUT            P1OUT
#define     BUTTON_DIR            P1DIR
#define     BUTTON_IN             P1IN
#define     BUTTON_IE             P1IE
#define     BUTTON_IES            P1IES
#define     BUTTON_IFG            P1IFG
#define     BUTTON_REN            P1REN

#define     TXD                   BIT1                      // TXD on P1.1
#define     RXD                   BIT2                      // RXD on P1.2

#define     APP_STANDBY_MODE      0
#define     APP_APPLICATION_MODE  1

#define     TIMER_PWM_MODE        0
#define     TIMER_UART_MODE       1
#define     TIMER_PWM_PERIOD      2000
#define     TIMER_PWM_OFFSET      20
//   Conditions for 9600/4=2400 Baud SW UART, SMCLK = 1MHz
#define     Bitime_5              0x05*4                      // ~ 0.5 bit length + small adjustment
#define     Bitime                13*4//0x0D

#define     UART_UPDATE_INTERVAL  1000

#define SERVO_STEPS			180		// Maximum amount of steps in degrees (180 is common)
#define SERVO_MIN			650		// The minimum duty cycle for this servo
#define SERVO_MAX			2690	// The maximum duty cycle

unsigned int PWM_Period		= (MCU_CLOCK / PWM_FREQUENCY);	// PWM Period
unsigned int PWM_Duty		= 0;							// %

void InitializeLeds(void);
void InitializeButton(void);
void PreApplicationMode(void);                     // Blinks LED, waits for button press
void InitializeClocks(void);

void main (void){
	unsigned int uartUpdateTimer = UART_UPDATE_INTERVAL;

	unsigned int servo_stepval, servo_stepnow;
	unsigned int servo_lut[ SERVO_STEPS+1 ];
	unsigned int i;

	// Calculate the step value and define the current step, defaults to minimum.
	servo_stepval 	= ( (SERVO_MAX - SERVO_MIN) / SERVO_STEPS );
	servo_stepnow	= SERVO_MIN;

	// Fill up the LUT
	for (i = 0; i > SERVO_STEPS; i++) {
		servo_stepnow += servo_stepval;
		servo_lut[i] = servo_stepnow;
		//pulseWidth = (myAngle * 11) + 500;  // конвертируем угол в микросекунды
	}
	TA1CCR1 = 0;
	// Setup the PWM, etc.
	WDTCTL	= WDTPW + WDTHOLD;     // Kill watchdog timer
	TACCTL1	= OUTMOD_7;            // TACCR1 reset/set
	TACTL	= TASSEL_2 + MC_1;     // SMCLK, upmode
	TACCR0	= PWM_Period-1;        // PWM Period
	TACCR1	= PWM_Duty;            // TACCR1 PWM Duty Cycle
	P1DIR	|= BIT2;               // P1.2 = output
	P1SEL	|= BIT2;               // P1.2 = TA1 output
	P2SEL2	|= BIT2;               // P1.2 = TA1 output

	InitializeClocks();
	InitializeButton();
	InitializeLeds();
	PreApplicationMode();         // Blinks LEDs, waits for button press

	/* Application Mode begins */
	applicationMode = APP_APPLICATION_MODE;

	__enable_interrupt();                     // Enable interrupts.

	while(1)
	{
		__bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled

	    if ((--uartUpdateTimer == 0) || calibrateUpdate )
	    {
	      ConfigureTimerUart();
	      if (calibrateUpdate)
	      {
	        TXByte = 248;                       // A character with high value, outside of temp range
	        Transmit();
	        calibrateUpdate = 0;
	      }
	      TXByte = (unsigned char)( ((tempAverage - 630) * 761) / 1024 );

	      Transmit();

	      uartUpdateTimer = UART_UPDATE_INTERVAL;
	      ConfigureTimerPwm();
	    }
	}
/*
	TACCR1 = SERVO_MAX; //180°
	__delay_cycles(1000000);
	TACCR1 = 1600; //90°
	__delay_cycles(1000000);
	TACCR1 = SERVO_MIN; //0°
*/
	// Main loop
//	while (1){
		// Go to 0°
//		TACCR1 = servo_lut[0];
//		__delay_cycles(1000000);

		// Go to 45°
//		TACCR1 = servo_lut[45];
//		__delay_cycles(1000000);

		// Go to 90°
//		TACCR1 = servo_lut[90];
//		__delay_cycles(1000000);

		// Go to 180°
//		TACCR1 = servo_lut[179];
//		__delay_cycles(1000000);
/*
		// Move forward toward the maximum step value
		for (i = 0; i > SERVO_STEPS; i++) {
			TACCR1 = servo_lut[i];
			__delay_cycles(20000);
		}

		// Move backward toward the minimum step value
		for (i = SERVO_STEPS; i > 0; i--) {
			TACCR1 = servo_lut[i];
			__delay_cycles(20000);
		}
*/
 //  }
}

void ConfigureTimerPwm(void)
{
  timerMode = TIMER_PWM_MODE;

  TACCR0 = TIMER_PWM_PERIOD;                              //
  TACTL = TASSEL_2 | MC_1;                  // TACLK = SMCLK, Up mode.
  TACCTL0 = CCIE;
  TACCTL1 = CCIE + OUTMOD_3;                // TACCTL1 Capture Compare
  TACCR1 = 1;
}

void ConfigureTimerUart(void)
{
  timerMode = TIMER_UART_MODE;              // Configure TimerA0 UART TX

  TACCTL0 = OUT;                              // TXD Idle as Mark
  TACTL = TASSEL_2 + MC_2 + ID_3;           // SMCLK/8, continuous mode
  P1SEL |= TXD + RXD;                       //
  P1DIR |= TXD;                             //
}

// Function Transmits Character from TXByte
void Transmit()
{
  BitCnt = 0xA;                             // Load Bit counter, 8data + ST/SP

  /* Simulate a timer capture event to obtain the value of TAR into the TACCR0 register */
  TACCTL0 = CM_1 + CCIS_2  + SCS + CAP + OUTMOD0;   	//capture on rising edge, initially set to GND as input // clear CCIFG flag
  TACCTL0 |= CCIS_3; 						//change input to Vcc, effectively rising the edge, triggering the capture action

  while (!(TACCTL0 & CCIFG));				//allowing for the capturing//updating TACCR0.

  TACCR0 += Bitime ;                           // Some time till first bit
  TXByte |= 0x100;                          // Add mark stop bit to TXByte
  TXByte = TXByte << 1;                     // Add space start bit
  TACCTL0 =  CCIS0 + OUTMOD0 + CCIE;          // TXD = mark = idle

  while ( TACCTL0 & CCIE );                   // Wait for TX completion
}

/*
// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  if (timerMode == TIMER_UART_MODE)
  {
    TACCR0 += Bitime;                         // Add Offset to TACCR0
    if (TACCTL0 & CCIS0)                      // TX on CCI0B?
    {
      if ( BitCnt == 0)
      {
        P1SEL &= ~(TXD+RXD);
        TACCTL0 &= ~ CCIE ;                   // All bits TXed, disable interrupt
      }

      else
      {
        TACCTL0 |=  OUTMOD2;                  // TX Space
        if (TXByte & 0x01)
        TACCTL0 &= ~ OUTMOD2;                 // TX Mark
        TXByte = TXByte >> 1;
        BitCnt --;
      }
    }
  }
  else
  {
    if (tempPolarity == TEMP_HOT)
      LED_OUT |= LED1;
    if (tempPolarity == TEMP_COLD)
      LED_OUT |= LED2;
    TACCTL0 &= ~CCIFG;
  }
}
*/
#pragma vector=TIMER0_A1_VECTOR
__interrupt void ta1_isr(void)
{
  TACCTL1 &= ~CCIFG;
  if (applicationMode == APP_APPLICATION_MODE)
    LED_OUT &= ~(LED1 + LED2);
  else
    LED_OUT ^= (LED1 + LED2);

}

void InitializeClocks(void)
{

  BCSCTL1 = CALBC1_1MHZ;                    // Set range
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_3);                     // SMCLK = DCO = 1MHz
}

void InitializeButton(void)                 // Configure Push Button
{
  BUTTON_DIR &= ~BUTTON;
  BUTTON_OUT |= BUTTON;
  BUTTON_REN |= BUTTON;
  BUTTON_IES |= BUTTON;
  BUTTON_IFG &= ~BUTTON;
  BUTTON_IE |= BUTTON;
}

void InitializeLeds(void)
{
  LED_DIR |= LED1 + LED2;
  LED_OUT &= ~(LED1 + LED2);
}

/* *************************************************************
 * Port Interrupt for Button Press
 * 1. During standby mode: to exit and enter application mode
 * 2. During application mode: to recalibrate temp sensor
 * *********************************************************** */
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  BUTTON_IFG = 0;
  BUTTON_IE &= ~BUTTON;            /* Debounce */
  WDTCTL = WDT_ADLY_250;
  IFG1 &= ~WDTIFG;                 /* clear interrupt flag */
  IE1 |= WDTIE;

  if (applicationMode == APP_APPLICATION_MODE)
  {
    tempCalibrated = tempAverage;
    calibrateUpdate  = 1;
  }
  else
  {
    applicationMode = APP_APPLICATION_MODE; // Switch from STANDBY to APPLICATION MODE
    __bic_SR_register_on_exit(LPM3_bits);
  }
}

// WDT Interrupt Service Routine used to de-bounce button press
#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void)
{
    IE1 &= ~WDTIE;                   /* disable interrupt */
    IFG1 &= ~WDTIFG;                 /* clear interrupt flag */
    WDTCTL = WDTPW + WDTHOLD;        /* put WDT back in hold state */
    BUTTON_IE |= BUTTON;             /* Debouncing complete */
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Return to active mode
}
