// interfacing 16x2 LCD for MSP430FR2355
// Original Author: Luan Ferreira Reis de Jesus 
// Modified for use on the FR2355 by: Peter Buckley

// interfacing 16x2 LCD for MSP430FR2355
// Author: Luan Ferreira Reis de Jesus (Modified for FR2355)

#include <msp430.h>

// define RS high
#define DR P1OUT |= BIT0

// define RS low
#define CR P1OUT &= ~BIT0

// define Read signal R/W = 1 for reading
#define READ P1OUT |= BIT1 

// define Write signal R/W = 0 for writing
#define WRITE P1OUT &= ~BIT1 

// define Enable high signal
#define ENABLE_HIGH P1OUT |= BIT2 

// define Enable Low signal
#define ENABLE_LOW P1OUT &= ~BIT2 

void configure_clocks()
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    
    // Configure clock for 1MHz operation
    CSCTL0 = CSKEY; // Unlock clock registers
    CSCTL1 = DCOFSEL_0; // Set DCO to 1MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK; // Select clock sources
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1; // Set all dividers to 1
    CSCTL0_H = 0; // Lock clock registers
}

void delay_us(unsigned int us)
{
    while (us--)
    {
        __delay_cycles(1); // 1 cycle per microsecond at 1MHz
    }
}

void delay_ms(unsigned int ms)
{
    while (ms--)
    {
        __delay_cycles(1000); // 1000 cycles per millisecond at 1MHz
    }
}

void data_write(void)
{
    ENABLE_HIGH;
    delay_ms(5);
    ENABLE_LOW;
}

void send_data(unsigned char data)
{
    unsigned char higher_nibble = 0xF0 & (data);
    unsigned char lower_nibble = 0xF0 & (data << 4);
    
    delay_us(200);
    
    WRITE;
    DR;
    
    // send higher nibble
    P1OUT = (P1OUT & 0x0F) | higher_nibble; 
    data_write();
    
    // send lower nibble
    P1OUT = (P1OUT & 0x0F) | lower_nibble; 
    data_write();
}

void send_string(char *s)
{
    while(*s)
    {
        send_data(*s);
        s++;
    }
}

void send_command(unsigned char cmd)
{
    unsigned char higher_nibble = 0xF0 & cmd;
    unsigned char lower_nibble = 0xF0 & (cmd << 4);
    
    WRITE;
    CR;
    
    // send higher nibble
    P1OUT = (P1OUT & 0x0F) | higher_nibble; 
    data_write();
    
    // send lower nibble
    P1OUT = (P1OUT & 0x0F) | lower_nibble; 
    data_write();
}

void lcd_init(void)
{
    P1DIR |= 0xFF;
    P1OUT &= 0x00;
    
    delay_ms(15);
    send_command(0x33);
    
    delay_us(200);
    send_command(0x32);
    
    delay_us(40);
    send_command(0x28); // 4-bit mode
    
    delay_us(40);
    send_command(0x0E); // Display on, cursor on
    
    delay_us(40);
    send_command(0x01); // Clear screen
    
    delay_us(40);
    send_command(0x06); // Increment cursor
    
    delay_us(40);
    send_command(0x80); // Set cursor to row 1 column 1
}

int main(void)
{
    configure_clocks();
    lcd_init();
    
    send_string("Hello world!");
    send_command(0xC0);
    send_string("MSP430FR2355 LCD");
    
    while (1)
    {
    }
}
