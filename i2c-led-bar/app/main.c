#include "intrinsics.h"
#include <msp430fr2310.h>
#include <stdbool.h>

// Led Variables
int stepIndex = 0;      // Current step index
int stepOldIndex[] = {0, 0, 0, 0, 0, 0, 0, 0};   // last index for each pattern
int prev_pattern = 0;
int stepStart = 0;      // Start of the selected pattern
int seqLength = 1;      // Length of sleected sequence
int basePeriod = 128;
int patternMultiplier = 1;
unsigned char stepSequence[] = {
                // Pattern 0
                0b10101010,
                0b10101010,
                // Pattern 1
                0b10101010,
                0b01010101,
                // Pattern 3
                0b00011000,
                0b00100100,
                0b01000010,
                0b10000001,
                0b01000010,
                0b00100100,
                // Pattern 5
                0b00000001,
                0b00000010,
                0b00000100,
                0b00001000,
                0b00010000,
                0b00100000,
                0b01000000,
                0b10000000,
                // Pattern 6
                0b01111111,
                0b10111111,
                0b11011111,
                0b11101111,
                0b11110111,
                0b11111011,
                0b11111101,
                0b11111110,
                // Pattern 7
                0b1,
                0b11,
                0b111,
                0b1111,
                0b11111,
                0b111111,
                0b1111111,
                0b11111111,
                // Pattern NULL
                0b0,
                0b0
            };

// I2C Slave on P1.3 (SCL) and P1.2 (SDA)
unsigned int RX_BUFF_SIZE = 1;
char Received[] = {0x0};
unsigned int Data_Cnt = 0;

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // heartbeat on P2.0
    P2OUT &= ~BIT0;
    P2DIR |= BIT0;

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    // enable interrupts
    __enable_interrupt();

    //-- Setup patterns
    setupLeds(); // NOTE: this doesn't seem to work if the LED bar is hooked up, so just pull it out, let it setup the pins, the drop it back in
    setPattern(7);

    // setup I2C Slave with SA=0x55
    //setupSlaveI2C(0x55, RX_BUFF_SIZE);

    while (true)
    {
        P2OUT ^= BIT0;

        // Delay for 100000*(1/MCLK)=0.1s
        __delay_cycles(100000);
    }
}

void setupLeds() {
    // Configure Leds (P1.1, P1.0, P2.7, P2.6, P1.4, P1.5, P1.6, P1.7)
    P1DIR |= BIT1 | BIT0 | BIT4 | BIT5 |BIT6 | BIT7;
    P2DIR |= BIT7 | BIT6;
    P1OUT &= ~(BIT1 | BIT0 | BIT4 | BIT5 |BIT6 | BIT7);
    P2OUT &= ~(BIT7 | BIT6);

    // Setup Timer 0 B3
    TB1CTL = TBSSEL__ACLK | MC__UP | TBCLR | ID__8; // SMCLK (1Mhz), Stop mode, clear timer, divide by 8
    TB1EX0 = TBIDEX__8 ;   // Extra division by 4
    TB1CCR0 = basePeriod;  // Set initial speed
    TB1CCTL0 |= CCIE;      // Enable compare interrupt
}


void setPattern(int a) {
    switch (a) {
        case 0:
            stepStart = 0;
            seqLength = 2;
            patternMultiplier = 4;
        break;
        
        case 1:
            stepStart = 2;
            seqLength = 2;
            patternMultiplier = 4;
        break;
        
        case 3:
            stepStart = 4;
            seqLength = 6;
            patternMultiplier = 2;
        break;
        
        case 5:
            stepStart = 10;
            seqLength = 8;
            patternMultiplier = 6;
        
        case 6:
            stepStart = 18;
            seqLength = 8;
            patternMultiplier = 2;
        break;

        case 7:
            stepStart = 26;
            seqLength = 8;
            patternMultiplier = 4;
        break;

        case 2:
        case 4:
        case 9:
            stepStart = 34;
            seqLength = 2;
            patternMultiplier = 4;
        break;
    }

    TB1CCR0 = basePeriod * patternMultiplier;
    
}

// Timer for Led bar
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB3_CCR0(void)
{
    // Update Leds (P1.1, P1.0, P2.7, P2.6, P1.4, P1.5, P1.6, P1.7)
    P1OUT = ((stepSequence[stepIndex + stepStart] <<1 ) & 0b10) |
            ((stepSequence[stepIndex + stepStart] >>1 ) & 0b1) |
            ((stepSequence[stepIndex + stepStart]) & 0b10000) |
            ((stepSequence[stepIndex + stepStart]) & 0b100000) |
            ((stepSequence[stepIndex + stepStart]) & 0b1000000) |
            ((stepSequence[stepIndex + stepStart]) & 0b10000000); // LSB (0) to P1.1 | 1 to P1.0 | 4 to P1.4 | 5 to P1.5 | 6 to P1.6 | 7 to P1.7
    P2OUT = ((stepSequence[stepIndex + stepStart] <<5 ) & 0b10000000) |
            ((stepSequence[stepIndex + stepStart] <<3 ) & 0b1000000);  // 2 to P2.7 | 3 to P2.6

    stepIndex = (stepIndex + 1) % seqLength; // Update step index
    TB1CCTL0 &= ~CCIFG;  // clear CCR0 IFG
}

/*
#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {
    // only for RXIFG0
    if (UCB0IV & USCI_I2C_UCRXIFG0) {
        Received[Data_Cnt] = UCB0RXBUF; // fill buffer
        if (Data_Cnt < sizeof(Received) - 1) {
            Data_Cnt++;
        } else {
            Data_Cnt = 0;
        }
        setPattern(Received[0]);
    }
}
*/