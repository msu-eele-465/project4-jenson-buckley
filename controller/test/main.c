#include <msp430fr2355.h>

// I2C Master Setup
void i2c_master_setup() {
    UCB0CTLW0 |= UCSWRST;                 // Hold USCI in reset

    //-- Setup I2C Pins
    P1SEL1 &= ~(BIT2 | BIT3);             // Clear SEL1 bits for SDA/SCL
    P1SEL0 |=  (BIT2 | BIT3);             // Set SEL0 bits for SDA/SCL

    //-- Configure I2C
    UCB0CTLW0 = UCSWRST | UCSSEL_3 | UCMODE_3 | UCMST | UCTR | UCSYNC; // SMCLK, I2C master, Tx mode
    UCB0BRW = 10;                         // SCL = SMCLK / 10 = 100kHz

    UCB0CTLW1 = UCASTP_2;                 // Auto STOP after TBCNT bytes

    PM5CTL0 &= ~LOCKLPM5;                 // Enable GPIOs

    UCB0CTLW0 &= ~UCSWRST;                // Release from reset

    __delay_cycles(10000);               // Setup settle delay
}

// I2C Send Function (1 byte to slave address 0x40)
void i2c_send(unsigned char data) {
    while (UCB0CTLW0 & UCTXSTP);          // Wait for STOP if needed
    UCB0I2CSA = 0x40;                     // Slave address

    UCB0TBCNT = 1;                        // Transmit 1 byte
    UCB0CTLW0 |= UCTXSTT;                // Generate START

    while (!(UCB0IFG & UCTXIFG0));       // Wait for TX buffer ready
    UCB0TXBUF = data;                    // Send data

    while (UCB0CTLW0 & UCTXSTP);         // Wait for STOP to finish
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;            // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                // Enable GPIOs

    __delay_cycles(50000);               // Power-on delay

    i2c_master_setup();                  // Setup I2C master
    __delay_cycles(10000);               // Short delay before sending

    i2c_send("Hello\n");                      // Send one byte to slave
    __delay_cycles(1000000);
    i2c_send("Goodbye\n");             

    while (1);                           // Loop forever (done)
}
