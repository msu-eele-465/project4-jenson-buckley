#include <msp430fr2310.h>
#include <string.h>

// Pin Definitions
#define LCD_RS BIT0     // Register Select
#define LCD_RW BIT6     // Read/Write
#define LCD_E  BIT7     // Enable
#define LCD_DATA P1OUT  // Data bus on Port 1

// I2C Varibles
#define I2C_SLAVE_ADDR 0x48
#define I2C_MSG_BYTES 0x1
volatile unsigned char received_data = 0;

// LCD Messages
char *messages[] = {
    "Static",
    "Toggle",
    "Up Counter",
    "In and Out",
    "Down Counter",
    "Rotate Left",
    "Rotate 7 Right",
    "Fill Left"
};

// Delay Function
void delay(unsigned int count) {
    while(count--) __delay_cycles(1000);
}

// Enable Pulse
void lcd_enable_pulse() {
    P2OUT |= LCD_E;
    delay(2);
    P2OUT &= ~LCD_E;
}

// Command Write Function
void lcd_write_command(unsigned char cmd) {
    enable_lcd();
    P2OUT &= ~LCD_RS;  // RS = 0 for command
    P2OUT &= ~LCD_RW;  // RW = 0 for write
    LCD_DATA = cmd;    // Write command to data bus
    lcd_enable_pulse();
    delay(50);         // Command execution delay
}

// Data Write Function
void lcd_write_data(unsigned char data) {
    enable_lcd();
    P2OUT |= LCD_RS;   // RS = 1 for data
    P2OUT &= ~LCD_RW;  // RW = 0 for write
    LCD_DATA = data;   // Write data to data bus
    lcd_enable_pulse();
    delay(50);         // Data write delay
}

// Clear LCD
void lcd_clear(){
    lcd_write_command(0x01); // Clear display
    delay(50);             // Delay for clear command
}

// LCD Initialization
void lcd_init() {
    enable_lcd();
    P2DIR |= LCD_RS | LCD_RW | LCD_E;
    P1DIR |= 0xFF;   // Set Port 1 as output for data bus

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

// Setup I2C Slave
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

    enable_i2c();

    UCB0CTLW0 &= ~UCSWRST;       // take out of SW reset
    UCB0IE |= UCRXIE0;          // enable Rx interrupts
}

// Switch to I2C Mode
void enable_i2c(){
    P1SEL1 &=~ BIT3;    // P1.3 = SCL
    P1SEL0 |= BIT3;
    P1SEL1 &=~ BIT2;    // P1.2 = SDA
    P1SEL0 |= BIT2;
}

//Switch to LCD Mode
void enable_lcd() {
    P1SEL0 &= ~(BIT2 | BIT3); // Disable I2C function
    P1DIR |= BIT2 | BIT3;     // Set as outputs for LCD
}

// Process Received I2C Data
void processReceivedData() {
    lcd_clear();  // Clear LCD
    switch (received_data) {
        case 0:
            lcd_display_string(messages[0]);
            break;
        case 1:
            lcd_display_string(messages[1]);
            break;
        case 2:
            lcd_display_string(messages[2]);
            break;
        case 3:
            lcd_display_string(messages[3]);
            break;
        case 4:
            lcd_display_string(messages[4]);
            break;
        case 5:
            lcd_display_string(messages[5]);
            break;
        case 6:
            lcd_display_string(messages[6]);
            break;
        default:
            lcd_display_string("Invalid Input");
            break;
    }
}

// Main Program
int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;     // Unlock GPIO ports

    lcd_init();               // Initialize LCD
    setupSlaveI2C(I2C_SLAVE_ADDR, I2C_MSG_BYTES); // Set up I2C slave

    __enable_interrupt();     // Enable global interrupts

    while (1) {
    }
}

// I2C Interrupt Service Routine
#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {
    received_data = UCB0RXBUF; // Read received byte
    processReceivedData();     // Update LCD
}