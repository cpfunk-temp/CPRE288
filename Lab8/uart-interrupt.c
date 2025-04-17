/*
*
*   uart-interrupt.c
*
*
*
*   @author
*   @date
*/

// The "???" placeholders should be the same as in your uart.c file.
// The "?????" placeholders are new in this file and must be replaced.

#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include "uart-interrupt.h"

#define BAUD_RATE 115200

// These variables are declared as examples for your use in the interrupt handler.
volatile char command_byte = -1; // byte value for special character used as a command
volatile int command_flag = 0; // flag to tell the main program a special command was received

void set_cmnd(char cmnd)
{
    command_flag = 0;
    command_byte = cmnd;
}

void uart_interrupt_init(void){
    //enable clock to GPIO port B
    SYSCTL_RCGCGPIO_R |= 0x02;

    //enable clock to UART1
    SYSCTL_RCGCUART_R |= 0x02;

    //wait for GPIOB and UART1 peripherals to be ready
    while ((SYSCTL_PRGPIO_R & 0x02) == 0);
    while ((SYSCTL_PRUART_R & 0x02) == 0);

    //enable alternate functions on port B pins
    GPIO_PORTB_AFSEL_R |= 0x03;

    //enable digital functionality on port B pins
    GPIO_PORTB_DEN_R |= 0x03;

    //enable UART1 Rx and Tx on port B pins
    GPIO_PORTB_PCTL_R &= ~0x0FF;
    GPIO_PORTB_PCTL_R |= 0x011;

    //calculate baud rate
    uint8_t ClkDiv = 16;
    uint32_t UARTSysClk = 16000000;
    uint16_t iBRD = UARTSysClk / (ClkDiv * BAUD_RATE);
    uint16_t fBRD = (uint16_t)((UARTSysClk % (ClkDiv * BAUD_RATE)) * 64.0f / (ClkDiv * BAUD_RATE) + 0.5);

    //turn off UART1 while setting it up
    UART1_CTL_R &= ~0x0301;

    //set baud rate
    //note: to take effect, there must be a write to LCRH after these assignments
    UART1_IBRD_R = iBRD;
    UART1_FBRD_R = fBRD;

    //set frame, 8 data bits, 1 stop bit, no parity, no FIFO
    //note: this write to LCRH must be after the BRD assignments
    UART1_LCRH_R = 0x060;

    //use system clock as source
    //note from the datasheet UARTCCC register description:
    //field is 0 (system clock) by default on reset
    //Good to be explicit in your code
    UART1_CC_R = 0x0;

  //////Enable interrupts

  //first clear RX interrupt flag (clear by writing 1 to ICR)
  UART1_ICR_R |= 0b00010000;

  //enable RX raw interrupts in interrupt mask register
  UART1_IM_R |= 0x10;

  //NVIC setup: set priority of UART1 interrupt to 1 in bits 21-23
  NVIC_PRI1_R = (NVIC_PRI1_R & 0xFF0FFFFF) | 0x00200000;

  //NVIC setup: enable interrupt for UART1, IRQ #6, set bit 6
  NVIC_EN0_R |= 0x40;

  //tell CPU to use ISR handler for UART1 (see interrupt.h file)
  //from system header file: #define INT_UART1 22
  IntRegister(INT_UART1, UART1_Handler);

  //globally allow CPU to service interrupts (see interrupt.h file)
  IntMasterEnable();

  //re-enable UART1 and also enable RX, TX (three bits)
  //note from the datasheet UARTCTL register description:
  //RX and TX are enabled by default on reset
  //Good to be explicit in your code
  //Be careful to not clear RX and TX enable bits
  //(either preserve if already set or set them)
  UART1_CTL_R |= 0x0301;

}

void uart_sendChar(char data){
    while (UART1_FR_R & 0x08);
    UART1_DR_R = data;
}

char uart_receive(void){
    //wait for byte received
    while((UART1_FR_R & 0x010) != 0);
    //return data
    return UART1_DR_R  & 0xFF;
}

void uart_sendStr(const char *str){
    while (*str != '\0')
    {
        uart_sendChar(*str);
        str++;
    }
}

// Interrupt handler for receive interrupts
void UART1_Handler()
{
    char byte_received;
    //check if handler called due to RX event
    if (UART1_MIS_R & 0x10)
    {
        //byte was received in the UART data register
        //clear the RX trigger flag (clear by writing 1 to ICR)
        UART1_ICR_R |= 0b00010000;

        //read the byte received from UART1_DR_R and echo it back to PuTTY
        //ignore the error bits in UART1_DR_R
        byte_received = UART1_DR_R & 0xFF;
        uart_sendChar(byte_received);

        //if byte received is a carriage return
        if (byte_received == '\r')
        {
            //send a newline character back to PuTTY
            uart_sendChar('\n');
        }
        else if (byte_received == command_byte)
        {
            command_flag = 1;
            return;
        }

        command_flag = 0;
    }
}
