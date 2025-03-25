#include <msp430fr2355.h>

/*
- assumes global vars as follows: int rPWM, int gPWM, int bPWM, int countPWM

- assumes an interrupt as follows:
    // Period for RGB PWM
    #pragma vector = TIMER3_B0_VECTOR
    __interrupt void ISR_PWM_PERIOD(void)
    {
        // RGB all on
        if (countPWM > rPWM) {
            P1OUT &= ~BIT5;
        } else {
            P1OUT |= BIT5;
        }
        if (countPWM > gPWM) {
            P1OUT &= ~BIT6;
        } else {
            P1OUT |= BIT6;
        }
        if (countPWM > bPWM) {
            P1OUT &= ~BIT7;
        } else {
            P1OUT |= BIT7;
        }
        if (countPWM > 255) {
            countPWM = 0;
        } else {
            countPWM += 1;
        }
        TB3CCTL0 &= ~CCIFG;  // clear CCR0 IFG
    }

NOTES:

- ensure global interrupts are enabled, low power mode is disabled, and watchdog timer is disabled:
__enable_interrupt();       // enable global interrupts
PM5CTL0 &=~ LOCKLPM5;       // disable low power mode
WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
*/

void setupRGBLED(void) {
    // outputs
    P1DIR |= BIT5;
    P1DIR |= BIT6;
    P1DIR |= BIT7;
    P1OUT &= ~BIT5;      
    P1OUT &= ~BIT6;
    P1OUT &= ~BIT7;
    // clock on TB3
    TB3CTL |= TBCLR;            // reset settings
    TB3CTL |= TBSSEL__SMCLK;    
    TB3CTL |= MC__UP;
    TB3CCR0 = 2;                // period (steps counter each time)
    TB3CCTL0 |= CCIE;           // Enable capture compare
    TB3CCTL0 &= ~CCIFG;         // Clear IFG
}