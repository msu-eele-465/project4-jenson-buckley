#include <msp430fr2355.h>
#include <stdbool.h>

// RGB LED colors
#define RGB_LOCKED 0xFF0000
#define RGB_UNLOCKING 0x0000FF
#define RGB_UNLOCKED 0x00FF00
#define RGB_PAT0 0x808080
#define RGB_PAT1 0xF0F0F0
#define RGB_PAT2 0xF0F0F0
#define RGB_PAT3 0x808080
#define RGB_PAT4 0xF0F0F0
#define RGB_PAT5 0x808080
#define RGB_PAT6 0xF0F0F0
#define RGB_PAT7 0x808080

// Setup RGB LED
setupRGBLED();
int rPWM = 0;
int gPWM = 0;
int bPWM = 0;
int countPWM = 0;

// TODO: setup I2C

// Setup keypad
setupKeypad();
char lastKey = 'X';

// STATE
// 0    Locked
// 1    First correct digit entered
// 2    Second
// 3    Third
// 4    Unlocked
int state = 0;

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    P1OUT &= ~BIT0;
    P1DIR |= BIT0;

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    while (true)
    {
        P1OUT ^= BIT0;

        char key_val = readKeypad();

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
                // lock
                if (key_val=='D') { 
                    state = 0;
                    updateHex(RGB_LOCKED);
                    // TODO: clear LCD command
                    // TODO: clead patterns command

                // otherwise
                } else {
                    else if (key_val=='0') {
                    updateHex(RGB_PAT0);
                    } else if (key_val=='1') {
                        updateHex(RGB_PAT1);
                    } else if (key_val=='2') {
                        updateHex(RGB_PAT2);
                    } else if (key_val=='3') {
                        updateHex(RGB_PAT3);
                    } else if (key_val=='4') {
                        updateHex(RGB_PAT4);
                    } else if (key_val=='5') {
                        updateHex(RGB_PAT5);
                    } else if (key_val=='6') {
                        updateHex(RGB_PAT6);
                    } else if (key_val=='7') {
                        updateHex(RGB_PAT7);
                    }
                    // TODO: send key press to LCD
                    // TODO: send key press to LED BAR
                }
                
            } else {
                state = 0;
                updateHex(RGB_LOCKED);
            }
        }
    }
}


// TODO: master I2C ISR

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