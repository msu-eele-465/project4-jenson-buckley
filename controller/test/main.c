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

// Integer send function (for led bar)
void i2c_send_int(unsigned char data) {
    while (UCB0CTLW0 & UCTXSTP);          // Wait for STOP if needed
    UCB0I2CSA = 0x40;                     // Slave address

    UCB0TBCNT = 1;                        // Transmit 1 byte
    UCB0CTLW0 |= UCTXSTT;                // Generate START

    while (!(UCB0IFG & UCTXIFG0));       // Wait for TX buffer ready
    UCB0TXBUF = data;                    // Send data

    while (UCB0CTLW0 & UCTXSTP);         // Wait for STOP to finish
}

// Messqage send function (for lcd)
void i2c_send_msg(const char *msg) {
    while (UCB0CTLW0 & UCTXSTP);  // Wait for previous STOP

    UCB0I2CSA = 0x40;             // Set slave address

    int len = 0;
    while (msg[len] != '\0') len++;  // Count characters

    UCB0TBCNT = len;              // Set transmit byte count
    UCB0CTLW0 |= UCTXSTT;         // Generate START

    int i;
    for (i = 0; i < len; i++) {
        while (!(UCB0IFG & UCTXIFG0));  // Wait for TXBUF ready
        UCB0TXBUF = msg[i];            // Send character
    }

    while (UCB0CTLW0 & UCTXSTP);  // Wait for STOP to complete
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;            // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                // Enable GPIOs

    __delay_cycles(50000);               // Power-on delay

    i2c_master_setup();                  // Setup I2C master
    __delay_cycles(10000);               // Short delay before sending

    //Example Integer Send for LED Bar

    //i2c_send_int(1);
    //__delay_cycles(1000000);
    //i2c_send_int(1);

    //Example Message Send for LED Bar

    i2c_send_msg("Hello\n");                      // Send one byte to slave
    __delay_cycles(1000000);
    i2c_send_msg("Goodbye\n");             

    while (1);                           // Loop forever (done)
}
