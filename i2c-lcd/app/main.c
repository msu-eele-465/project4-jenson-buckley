#include <msp430fr2355.h>

// Pin Definitions
#define LCD_RS BIT0     // Register Select
#define LCD_RW BIT1     // Read/Write
#define LCD_E  BIT2     // Enable
#define LCD_DATA P2OUT  // Data bus on Port 1

// Delay Function
void delay(unsigned int count) {
    while(count--) __delay_cycles(1000);
}

// Enable Pulse
void lcd_enable_pulse() {
    P3OUT |= LCD_E;
    delay(2);
    P3OUT &= ~LCD_E;
}

// Command Write Function
void lcd_write_command(unsigned char cmd) {
    P3OUT &= ~LCD_RS;  // RS = 0 for command
    P3OUT &= ~LCD_RW;  // RW = 0 for write
    LCD_DATA = cmd;    // Write command to data bus
    lcd_enable_pulse();
    delay(50);         // Command execution delay
}

// Data Write Function
void lcd_write_data(unsigned char data) {
    P3OUT |= LCD_RS;   // RS = 1 for data
    P3OUT &= ~LCD_RW;  // RW = 0 for write
    LCD_DATA = data;   // Write data to data bus
    lcd_enable_pulse();
    delay(50);         // Data write delay
}

// LCD Initialization
void lcd_init() {
    P3DIR |= LCD_RS | LCD_RW | LCD_E;
    P2DIR |= 0xFF;   // Set Port 1 as output for data bus

    delay(50);    // Power-on delay

    lcd_write_command(0x38); // Function set: 8-bit, 2 lines, 5x8 dots
    lcd_write_command(0x0C); // Display ON, Cursor OFF
    lcd_clear();
    lcd_write_command(0x06); // Entry mode set: Increment cursor
}

// Display String on LCD
void lcd_display_string(char *str) {
    while(*str) {
        lcd_write_data(*str++);
    }
}

// Clear LCD
void lcd_clear(){
    lcd_write_command(0x01); // Clear display
    delay(50);             // Delay for clear command
}

void i2c_slave_setup(){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    UCB0CTLW0 |= UCSWRST;       // put eUSC_B0 into software reset

    UCB0CTLW0 |= UCMODE_3;      // put into I2C mode
    UCB0CTLW0 &= ~UCMST;        // put into slave mode
    UCB0I2COA0 = 0x0040;        // define slave address
    UCB0I2COA0 |= UCOAEN;       // enable slave address
    UCB0CTLW0 &= ~UCTR;         // put into Rx mode
    UCB0CTLW1 &= ~UCASTP0;      // disable automatic stop condition
    UCB0CTLW1 &= ~UCASTP1;

    //-- Setup P1.3 as SCL and P1.2 as SDA
    P1SEL1 &= ~BIT3;            // SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;            // SDA
    P1SEL0 |= BIT2;

    PM5CTL0 &= ~LOCKLPM5;       // Turn on GPIO

    UCB0CTLW0 &= ~UCSWRST;      // take out of SW reset

    UCB0IE |= UCRXIE0;          // enable Rx interrupt
}

// Main Program
int main(void) {
    i2c_slave_setup();          // Setup I2C
    lcd_init();                 // Initialize LCD
    __enable_interrupt();       // Enable global iunterupts

    while (1) {
    }
}

// I2C Interrupt Service Routine
#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {
    int received_data = UCB0RXBUF; // Read received byte
    
    lcd_clear();  // Clear LCD
    switch (received_data) {
        case 0:
            lcd_display_string("Static");
            break;
        case 1:
            lcd_display_string("Toggle");
            break;
        case 2:
            lcd_display_string("Up Counter");
            break;
        case 3:
            lcd_display_string("In and Out");
            break;
        case 4:
            lcd_display_string("Down Counter");
            break;
        case 5:
            lcd_display_string("Rotate Left");
            break;
        case 6:
            lcd_display_string("Rotate 7 Right");
            break;
        default:
            lcd_display_string("Invalid Input");
            break;
    }
}