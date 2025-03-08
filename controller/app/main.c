#include <msp430fr2355.h>
#include <stdbool.h>

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    P1OUT &= ~BIT0;
    P1DIR |= BIT0;

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    // TODO: setup RGB LED

    // TODO: setup I2C

    // Setup keypad
    setupKeypad();

    // STATE
    // 0    Locked
    // 1    First correct digit entered
    // 2    Second
    // 3    Third
    // 4    Unlocked
    int state = 0;

    while (true)
    {
        P1OUT ^= BIT0;

        char key_val = readKeypad();

        if (key_val != 'X') {
            if (state == 0) {
                if (key_val=='1') {
                    state = 1;
                    // TODO: unlocking PWM color
                } else {
                    state = 0;
                }

            } else if (state == 1) {
                if (key_val=='1') {
                    state = 2;
                } else {
                    state = 0;
                    // TODO: locked PWM color
                }

            } else if (state == 2) {
                if (key_val=='1') {
                    state = 3;
                } else {
                    state = 0;
                    // TODO: locked PWM color
                }
                
            } else if (state == 3) {
                if (key_val=='1') {
                    state = 4;
                    // TODO: unlocked PWM color
                } else {
                    state = 0;
                    // TODO: locked PWM color
                }

            } else if (state == 4) {
                // lock
                if (key_val=='D') { 
                    state = 0;
                    // TODO: locked PWM color
                    // TODO: clear LCD command
                    // TODO: clead patterns command

                // anything else
                } else {
                    // TODO: send key press to LCD
                    // TODO: send key press to LED BAR
                } 
            } else {
                state = 0;
                // TODO: locked PWM color
            }
        }
    }
}


// TODO: master I2C ISR

// TODO: RGB LED PWM