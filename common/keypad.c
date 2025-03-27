#include <msp430fr2355.h>

/*
NOTES: Assumes global char lastKey initialized to  'X'

- ensure global interrupts are enabled, low power mode is disabled, and watchdog timer is disabled:
__enable_interrupt();       // enable global interrupts
PM5CTL0 &=~ LOCKLPM5;       // disable low power mode
WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
*/

/*
Setup the keypad with the columns as output and rows as inputs.
    ...rows: P1.4, P5.3, P5.1, P5.0
    ...columns:    P5.4, P1.1, P3.5, 3.1
*/
void setupKeypad() {
    // rows as outputs on P1.4, P5.3, P5.1, P5.0 initialized to 0
    P1DIR |= BIT4;
    P5DIR |= BIT3;
    P5DIR |= BIT1;
    P5DIR |= BIT0;
    P1OUT &= ~BIT4;      
    P5OUT &= ~BIT3;
    P5OUT &= ~BIT1;
    P5OUT &= ~BIT0;
    // cols as inputs pulled down internally on P5.4, P1.1, P3.5, 3.1
    P5DIR &= ~BIT4;     // inputs
    P1DIR &= ~BIT1;
    P3DIR &= ~BIT5;
    P3DIR &= ~BIT1;
    P5REN |= BIT4;      // internal resistors
    P1REN |= BIT1;
    P3REN |= BIT5;
    P3REN |= BIT1;
	P5OUT &=~ BIT4;     // pull-downs
	P1OUT &=~ BIT1;
	P3OUT &=~ BIT5;
	P3OUT &=~ BIT1;
}

/*
Check for inputs for cols 1-4 (P5.4, P1.1, P3.5, 3.1).

Returns 0-3 for a pressed col (first pressed col) or -1 if nothing is pressed
*/
int checkCols() {
    if (P5IN & BIT4) {
        return 0;
    } else if (P1IN & BIT1) {
        return 1;
    } else if (P3IN & BIT5) {
        return 2;
    } else if (P3IN & BIT1) {
        return 3;
    } else {
        return -1;
    }
}


/*
Read keypad input and return the corrisponding character ('X' for nothing)
*/
char readKeypad(lastKey) {
    // columns on P1.4, P5.3, P5.1, P5.0
    // rows on P5.4, P1.1, P3.5, 3.1

    char keys[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    char pressed = 'X';

    // check row 1
    P1OUT |= BIT4;
    int col = checkCols();
    if (col!=-1) {
        pressed = keys[0][col];
    } 
    P1OUT &= ~BIT4;

    // check row 2
    P5OUT |= BIT3;
    col = checkCols();
    if (col!=-1) {
        pressed =  keys[1][col];
    } 
    P5OUT &= ~BIT3;

    // check row 3
    P5OUT |= BIT1;
    col = checkCols();
    if (col!=-1) {
        pressed =  keys[2][col];
    }
    P5OUT &= ~BIT1;

    // check row 4
    P5OUT |= BIT0;
    col = checkCols();
    if (col!=-1) {
        pressed =  keys[3][col];
    }
    P5OUT &= ~BIT0;

    if (pressed != lastKey) {
        lastKey = pressed;
        return pressed;
    } else {
        return 'X';
    }
}