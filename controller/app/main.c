#include <msp430fr2355.h>
#include <stdbool.h>
#include "intrinsics.h"
#include "keypad.h"
#include "rgb_led.h"

// RGB LED colors
#define RGB_LOCKED 0xFF0000
#define RGB_UNLOCKING 0x0000FF
#define RGB_UNLOCKED 0x00FF00
#define RGB_PAT0 0x808080
#define RGB_PAT1 0xF0F0F0
#define RGB_PAT2 0xF0F000
#define RGB_PAT3 0x808000
#define RGB_PAT4 0xF000F0
#define RGB_PAT5 0x800080
#define RGB_PAT6 0x00F0F0
#define RGB_PAT7 0x008080

// Setup RGB LED
int rPWM;
int gPWM;
int bPWM;
int countPWM;
void updateHex(int);

// Setup keypad
char lastKey = 'X';

// STATE
// 0    Locked
// 1    First correct digit entered
// 2    Second
// 3    Third
// 4    Unlocked
int state = 0;

// I2C Master Setup
void i2c_master_setup(){
    UCB0CTLW0 |= UCSWRST;   // Hold USCI in reset

    //-- Setup I2C Pins
    P1SEL1 &= ~(BIT2 | BIT3);   
    P1SEL0 |=  (BIT2 | BIT3);             

    //-- Configure I2C
    UCB0CTLW0 = UCSWRST | UCSSEL_3 | UCMODE_3 | UCMST | UCTR | UCSYNC; // SMCLK, I2C master, Tx mode
    UCB0BRW = 10;                   // SCL = SMCLK / 10 = 100kHz

    UCB0CTLW1 = UCASTP_2;           // Auto STOP after TBCNT bytes

    PM5CTL0 &= ~LOCKLPM5;           // Enable GPIOs

    UCB0CTLW0 &= ~UCSWRST;          // Release from reset

    __delay_cycles(10000);          // Setup settle delay
}

// Integer send function (for led bar)
void i2c_send_int(unsigned char data) {
    while (UCB0CTLW0 & UCTXSTP);          // Wait for STOP if needed
    UCB0I2CSA = 0x45;                     // Slave address

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

// Update RGB color
void updateHex(int hex_code) {
    rPWM = 0xFF & (hex_code >> 4);
    gPWM = 0xFF & (hex_code >> 2);
    bPWM = 0xFF & hex_code;
}

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;   // Disable low-power mode 
    
    PM5CTL0 &= ~LOCKLPM5;       // GPIO high-impedance

    __enable_interrupt();       // enable interrupts

    __delay_cycles(50000);      // Power-on delay

    // Setup
    rPWM = 0;
    gPWM = 0;
    bPWM = 0;
    countPWM = 0;

    i2c_master_setup();
    setupRGBLED();
    setupKeypad();
    char message[] = {0x0};

    i2c_send_int(9);        // initialze LED bar to off
    //i2c_send_msg("\n");     // initialze LCD to off
    updateHex(RGB_LOCKED);  // RGB LED to locked color

    while (true)
    {
        
        char key_val = readKeypad(lastKey);

        if (key_val != 'X') {
            
            if (state == 0) {
                
                if (key_val=='1') {
                    state = 1;
                    updateHex(RGB_UNLOCKING);
                } else {
                    state = 0;
                }

            } else if (state == 1) {
                if (key_val=='1') {
                    state = 2;
                } else {
                    state = 0;
                    updateHex(RGB_LOCKED);
                }

            } else if (state == 2) {
                if (key_val=='1') {
                    state = 3;
                } else {
                    state = 0;
                    updateHex(RGB_LOCKED);
                }
                
            } else if (state == 3) {
                if (key_val=='1') {
                    state = 4;
                    updateHex(RGB_UNLOCKED);
                } else {
                    state = 0;
                    updateHex(RGB_LOCKED);
                }

            } else if (state == 4) {
                char keypress[] = {key_val, "\n"};
                //i2c_send_msg(keypress);       // send key press

                // lock
                if (key_val=='D') { 
                    state = 0;
                    updateHex(RGB_LOCKED);
                    //i2c_send_msg("\n");      // clear LCD
                    i2c_send_int(9);        // clear LED bar
                
                // inc base period
                } else if (key_val=='A') { 
                    i2c_send_int(11);        // inc base period

                } else if (key_val=='B') { 
                    i2c_send_int(10);        // inc base period
                
                // otherwise
                } else {
                    if (key_val=='0') {
                        // turn led bar to pattern 0
                        //i2c_send_msg("static\n");    // update LCD
                        i2c_send_int(0);            // update LED bar
                        updateHex(RGB_PAT0);
                    } else if (key_val=='1') {
                        // turn led bar to pattern 1
                        //i2c_send_msg("toggle\n");    // update LCD
                        i2c_send_int(1);            // update LED bar
                        updateHex(RGB_PAT1);
                    } else if (key_val=='2') {
                        // turn led bar to pattern 2
                        //i2c_send_msg("up counter\n");    // update LCD
                        i2c_send_int(2);            // update LED bar
                        updateHex(RGB_PAT2);
                    } else if (key_val=='3') {
                        // turn led bar to pattern 3
                        message[0] = 3;
                        //i2c_send_msg("in and out\n");    // update LCD
                        i2c_send_int(3);            // update LED bar
                        updateHex(RGB_PAT3);
                    } else if (key_val=='4') {
                        // turn led bar to pattern 4
                        //i2c_send_msg("down counter\n");    // update LCD
                        i2c_send_int(4);            // update LED bar
                        updateHex(RGB_PAT4);
                    } else if (key_val=='5') {
                        // turn led bar to pattern 5
                        //i2c_send_msg("ritate 1 left\n");    // update LCD
                        i2c_send_int(5);            // update LED bar
                        updateHex(RGB_PAT5);
                    } else if (key_val=='6') {
                        // turn led bar to pattern 6
                        //i2c_send_msg("rotate 7 right\n");    // update LCD
                        i2c_send_int(6);            // update LED bar
                        updateHex(RGB_PAT6);
                    } else if (key_val=='7') {
                        // turn led bar to pattern 7
                        //i2c_send_msg("fill left\n");    // update LCD
                        i2c_send_int(7);            // update LED bar
                        updateHex(RGB_PAT7);
                    }
                }

            } else {
                state = 0;
                //i2c_send_msg("\n");      // clear LCD
                i2c_send_int(9);        // clear LED bar
                updateHex(RGB_LOCKED);
            }
        }
    }
}


// RGB LED PWM Control
#pragma vector = TIMER3_B0_VECTOR
__interrupt void ISR_PWM_PERIOD(void)
{
    // update outputs
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
    // update count
    if (countPWM > 255) {
        countPWM = 0;
    } else {
        countPWM += 1;
    }

    // clear CCR0 IFG
    TB3CCTL0 &= ~CCIFG;
}