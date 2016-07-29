//
// Timer_A PWM example for MSP430G2553
//
// This program generates a 1kHz PWM signal on TA1.1 (pin 10).
// The duty cycle ramps up steadily from 0% to 100% repeatedly.
// This is achieved simply by incrementing the value of TA1CCR1.
// The PWM period could also be changed by assigning a new value
// to TA1CCR0, but that isn't done here.
// 
// An LED driven by TA1.1 (pin 10) steadily increases in
// brightness from completely off to fully on over a two-
// second period. This repeats indefinitely. 
//
// Written by Ted Burke - last updated 12-2-2014
//
  
#include <msp430.h>
 
int main( void )
{
    // Watchdog timer
    WDTCTL = WDTPW + WDTHOLD; // Disable watchdog timer
     
    // Select primary peripheral module function on P2.2 (i.e. TA1.1)
    P2SEL  = 0b00000100;
    P2SEL2 = 0b00000000;
    P2DIR  = 0b00000100; // Pin must also be configured as an output
     
    // Configure Timer A interrupt
    TA1CTL = TASSEL_2 + ID_0 + MC_1; // Timer_A1: SMCLK clock, input divider=1, "up" mode
    TA1CCR0 = 1000;                  // Set Timer_A1 period to 1ms for 1kHz PWM
    TA1CCR1 = 500;                   // 50% duty cycle initially (500us pulse width)
    TA1CCTL1 = OUTMOD_7;             // Select "Reset/Set" output mode
     
    // This loop is just an example of varying the PWM duty cycle on
    // TA1.1 simply by updating the value of TA1CCR1.
    while(1)
    {
        TA1CCR1 += 1;
        if (TA1CCR1 >= TA1CCR0) TA1CCR1 = 0;
        __delay_cycles(2000);
    }
     
    return 0;
}