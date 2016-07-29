// Analog signal record and transmit for MSP430G2553
// Samples pins A3,A4,A5 at approx 10Hz; prints values via UART
// Written by Ted Burke - last updated 12-2-2014
//

#include <msp430.h>
#include <stdio.h>

// Function prototypes
void setup();
int read_analog_channel(unsigned int);

int main( void )
{
    setup(); // configure MSP430 pins

    int v1, v2, v3;

    while(1)
    {
        // Read voltages from analog inputs A3, A4, A5
        v1 = read_analog_channel(3);
        v2 = read_analog_channel(4);
        v3 = read_analog_channel(5);

        // Print the three measurements via the UART
        P1OUT = 1; // LED on
        printf("%04d,%04d,%04d\n", v1, v2, v3);
        P1OUT = 0; // LED off

        __delay_cycles(100000); // 100ms delay
    }

    return 0;
}

//
// This function configures the pins and modules of the MSP430
//
void setup()
{
    // Watchdog timer
    WDTCTL = WDTPW + WDTHOLD; // Disable watchdog timer

    // Digital i/o
    P2DIR = 0b00000000; // P2.0-7 are inputs
    P1DIR = 0b00000001; // P1.0 is an output, P1.1-7 are inputs

    // Analog inputs
    ADC10AE0 = 0b00111000; // A3,A4,A5 (pins 5-7) are analog inputs
    ADC10CTL0 = ADC10ON;   // Turn on the ADC

    // Basic Clock Module (set to 1MHz)
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;

    // UART
    // Baudrate = 1MHz / (256 * UCA0BR1 + UCA0BR0)
    UCA0BR1 = 0; UCA0BR0 = 104;  // Set baudrate = 9600
    UCA0CTL1 |= UCSSEL_2;        // Set USCI clock to SMCLK
    UCA0MCTL = UCBRS0;           // Modulation UCBRSx = 1
    P1SEL = BIT2; P1SEL2 = BIT2; // Set P1.2 as TXD
    UCA0CTL1 &= ~UCSWRST;        // Start USCI (release from reset)
}

//
// For the printf function (from stdio.h) to work, we need to provide
// a putchar function which transmits a single character via the UART.
//
int putchar(int c)
{
    UCA0TXBUF = c;
    while((IFG2 & UCA0TXIFG) == 0);
}

//
// This function performs a single analog to digital conversion,
// converting the voltage on analog input pin ANx into a 10-bit
// unsigned integer. Execution time for this function is of the
// order of 100us.
//
int read_analog_channel(unsigned int x)
{
    ADC10CTL0 &= ~ENC;            // disable conversion
    ADC10CTL1 = x << 12;          // select channel
    ADC10CTL0 |= ENC;             // enable conversion
    ADC10CTL0 |= ADC10SC;         // start conversion
    while(ADC10CTL1 & ADC10BUSY); // wait until complete
    return ADC10MEM;              // return digital value
}
