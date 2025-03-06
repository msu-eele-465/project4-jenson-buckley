#include <msp430fr2355.h>
#include <stdbool.h>
#include "i2c.h"

char tx_buff[100];
unsigned int index = 0;

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    P1OUT &= ~BIT0;
    P1DIR |= BIT0;

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    // enable global interrupts
    __enable_interrupt();

    // setup master I2C on P1.2 (SDA) and P1.3 (SCL)
    setupMasterI2C();

    while (true)
    {
        char message[] = {"test"};
        Tx(0x55, message, tx_buff);
        P1OUT ^= BIT0;
        // Delay for 100000*(1/MCLK)=0.1s
        __delay_cycles(100000);
    }
}

#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {
    UCB0TXBUF = tx_buff[index];     // runs when TX is ready for data
                                    // happens when ACK is received
    if (index < sizeof(tx_buff) - 1) { index++; }
    else { index = 0; }
}