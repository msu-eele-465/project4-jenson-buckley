#include <msp430fr2355.h>

/*
NOTES:

 - this assumes that an ISR is included as follows for a MASTER (which references a global character array tx_buff and global int index and global int message_length):
#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {
    UCB0TXBUF = tx_buff[index];     // runs when TX is ready for data
                                    // happens when ACK is received
    if (index < sizeof(tx_buff) - 1) { index++; }
    else { index = 0; }
}

 - this assumes that an ISR is included as follows for a SLAVE (which references a global character array Received and global int Data_Cnt)
#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {
    Received[Data_Cnt] = UCB0RXBUF; // fill buffer
        if (Data_Cnt < sizeof(Received) - 1) {
            Data_Cnt++;
        } else {
            Data_Cnt = 0;
        }
}

- before calling setup functions, ensure to put the MSP into software reset mode (and then take it out after):
UCB0CTLW0 |= UCSWRST;       // put into software reset
UCB0CTLW0 &=~ UCSWRST;      // take out of software reset

- ensure global interrupts are enabled, low power mode is disabled, and watchdog timer is disabled:
__enable_interrupt();       // enable global interrupts
PM5CTL0 &=~ LOCKLPM5;       // disable low power mode
WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
*/


/*
Setup a 100kHz I2C Master on P1.3 (SCL) and P1.2 (SDA). Note that slave address and message length are NOT set.
*/
void setupMasterI2C() {
    UCB0CTLW0 |= UCSWRST;       // put into SW reset

    UCB0CTLW0 |= UCSSEL_3;      // choose BRCLK=SMCLK=1MHz
    UCB0BRW = 10;               // divide by 10 for SCL=100kHz

    UCB0CTLW0 |= UCMODE_3;      // put into I2C mode
    UCB0CTLW0 |= UCMST;         // put into master mode
    UCB0CTLW0 |= UCTR;          // put into tx mode
    UCB0CTLW1 |= UCASTP_2;      // auto stop when UCB0TBCNT

    P1SEL1 &=~ BIT3;    // P1.3 = SCL
    P1SEL0 |= BIT3;
    P1SEL1 &=~ BIT2;    // P1.2 = SDA
    P1SEL0 |= BIT2;

    UCB0CTLW0 &= ~UCSWRST;       // take out of SW reset

}


/*
Set the slave address, message length, enable interrupts, and generate start condition. Call to transmit a message.
*/
void Tx(int slave_addr, char message[], char tx_buff[], int *message_length) {
    tx_buff = message;       // update TX buffer
    UCB0I2CSA = slave_addr;         // set slave address
    UCB0TBCNT = sizeof(message);    // send all bytes of data
    *message_length = sizeof(message);
    UCB0IE |= UCTXIE0;              // enable TX interrupt
    UCB0CTLW0 |= UCTXSTT;           // generate start condition
}


/*
Setup a 100kHz I2C slave on P1.3 (SCL) and P1.2 (SDA). Pass slave address (7 bits) and number of bytes to receive per Rx.
Note the message length is NOT set.
*/
void setupSlaveI2C(int slave_addr, int bytes) {
    UCB0CTLW0 |= UCSWRST;       // put into SW reset

    UCB0CTLW0 |= UCMODE_3;      // put into I2C mode
    UCB0CTLW0 &= ~UCMST;        // put into slave mode
    UCB0CTLW0 |= UCMODE0;       // put into slave mode
    UCB0CTLW0 |= UCSYNC;        // put into slave mode
    UCB0CTLW0 &= ~UCTR;         // put into rx mode
    UCB0I2COA0 &= ~0x87FF;      // reset the slave address register (don't touch bits 14-11 since they are reserved)
    UCB0I2COA0 |= slave_addr;   // set slave address
    UCB0I2COA0 |= UCOAEN;       // enable interrupts for slave address

    UCB0CTLW1 |= UCASTP_2;      // auto stop when UCB0TBCNT
    UCB0TBCNT = bytes;          // receive all bytes of data

    P1SEL1 &=~ BIT3;    // P1.3 = SCL
    P1SEL0 |= BIT3;
    P1SEL1 &=~ BIT2;    // P1.2 = SDA
    P1SEL0 |= BIT2;

    UCB0CTLW0 &= ~UCSWRST;       // take out of SW reset
    UCB0IE |= UCRXIE0;          // enable Rx interrupts
}