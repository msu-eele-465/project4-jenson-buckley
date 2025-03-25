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

    P1OUT &= ~BIT0;
    P1DIR |= BIT0;

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    //-- Setup patterns
    setupLeds();
    setPattern(9);

    // setup I2C Slave with SA=0x55
    setupSlaveI2C(0x55, RX_BUFF_SIZE);

    while (true)
    {
        P1OUT ^= BIT0;

        // Delay for 100000*(1/MCLK)=0.1s
        __delay_cycles(100000);
    }
}

void setupLeds() {
    // Configure Leds (P6.0 - P6.4, P2.0 - P2.2)
    P6DIR |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4;
    P6OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4);
    P2DIR |= BIT0 | BIT1 | BIT2;
    P2OUT &= ~(BIT0 | BIT1 | BIT2);

    // Setup Timer B1
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
            updateRedPWM(0xFF);
            updateGreenPWM(0x00);
            updateBluePWM(0x00);
        break;
        
        case 1:
            stepStart = 2;
            seqLength = 2;
            patternMultiplier = 4;
            updateRedPWM(0x00);
            updateGreenPWM(0xFF);
            updateBluePWM(0x00);
        break;
        
        case 3:
            stepStart = 4;
            seqLength = 6;
            patternMultiplier = 2;
            updateRedPWM(0x00);
            updateGreenPWM(0x00);
            updateBluePWM(0xFF);
        break;
        
        case 5:
            stepStart = 10;
            seqLength = 8;
            patternMultiplier = 6;
            updateRedPWM(0xFF);
            updateGreenPWM(0xFF);
            updateBluePWM(0x00);
        break;
        
        case 6:
            stepStart = 18;
            seqLength = 8;
            patternMultiplier = 2;
            updateRedPWM(0xFF);
            updateGreenPWM(0x00);
            updateBluePWM(0xFF);
        break;

        case 7:
            stepStart = 26;
            seqLength = 8;
            patternMultiplier = 4;
            updateRedPWM(0x00);
            updateGreenPWM(0xFF);
            updateBluePWM(0xFF);
        break;

        case 2:
        case 4:
        case 9:
            stepStart = 34;
            seqLength = 2;
            patternMultiplier = 4;
            updateRedPWM(0x00);
            updateGreenPWM(0x00);
            updateBluePWM(0x00);
        break;
    }

    TB0CCR0 = basePeriod * patternMultiplier;
    
}

// Timer for Led bar
#pragma vector = TIMER0_B1_VECTOR
__interrupt void ISR_TB1_CCR0(void)
{
    P6OUT = (stepSequence[stepIndex + stepStart] & 0b00011111); // Update Led output
    P2OUT = ((stepSequence[stepIndex + stepStart] >>5 ) & 0b00000111); // Update Led output
    stepIndex = (stepIndex + 1) % seqLength; // Update step index
    TB1CCTL0 &= ~CCIFG;  // clear CCR0 IFG
}

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