#include <msp430fr2355.h>
#include <stdbool.h>
#include "i2c.h"

/* MASTER
char tx_buff[100];
unsigned int index = 0;
unsigned int message_length = 0;
*/

/* SLAVE */
unsigned int RX_BUFF_SIZE = 4;
char Received[] = {0x0, 0x0, 0x0, 0x0};
unsigned int Data_Cnt = 0;

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
    //setupMasterI2C();
    setupSlaveI2C(0x68, RX_BUFF_SIZE);

    while (true)
    {
        /* MASTER TEST
        char message[] = {0xFF, 0xF, 0x55};
        Tx(0x55, message, tx_buff, &message_length);
        P1OUT ^= BIT0;
        // Delay for 100000*(1/MCLK)=0.1s
        __delay_cycles(100000);
        */        
    }
}
/* MASTER
#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {
    UCB0TXBUF = tx_buff[index];     // runs when TX is ready for data
                                    // happens when ACK is received
    if (index < message_length - 1) { index++; }
    else { index = 0; }
}
*/

/* SLAVE */
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
    }
}