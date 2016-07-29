//
// Servo control PWM example for MSP430G2553
//
// This program generates a 50Hz PWM signal on TA1.1 (pin 10),
// suitable for servo control. In my servo, pulse widths vary
// between 1ms (0 degrees) and 2ms (180 degrees).
//
// The main loop just cycles indefinitely through three servo
// angles.
//
// Written by Ted Burke - last updated 28-3-2014
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
    TA1CCR0 = 20000;                 // Set PWM period to 20ms for servo control
    TA1CCR1 = 1500;                  // Set initial pulse width to 1.5ms
    TA1CCTL1 = OUTMOD_7;             // Select "Reset/Set" output mode

    // This loop cycles the servo through three angle steps
    // In my servo, as in many low-cost angle servos, the
    // pulse width is controlled as follows (approximately):
    //
    // - pulse widths should be in the range 1-2ms
    // - 1ms is 0 degrees
    // - 2ms is 180 degrees
    //
    while(1)
    {
        TA1CCR1 = 1200;          // 36 degrees approx
        __delay_cycles(1000000); // 1 second delay
        TA1CCR1 = 1500;          // 90 degrees approx
        __delay_cycles(1000000); // 1 second delay
        TA1CCR1 = 1800;          // 144 degrees approx
        __delay_cycles(1000000); // 1 second delay
    }

    return 0;
}
