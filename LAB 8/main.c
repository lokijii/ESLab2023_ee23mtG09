/*
 * UART communication with a baud rate of 9600 and odd parity.
 * Transmit "F0" when SW1 is pressed.
 * Transmit "AA" when SW2 is pressed.
 * If "AA" is received, turn on the LED in GREEN.
 * If "F0" is received, turn on the LED in BLUE.
 * If any UART error is detected, turn on the LED in RED.
 *
 * Configuration for using pins PE4 (Rx) and PE5 (Tx) with UART Module 5.
 */

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

// Bit masks for user switches and data values
#define Sw_Bits 0x11
#define S1_data 0xF0
#define S2_data 0xAA

// Bit masks for LED colors
#define Red 0X02
#define Blue 0X04
#define Green 0X08

// Function declarations
void PortF_Config(void);
void UART_Config(void);
void PortF_Handler(void);
void UART_Handler(void);
void Systick_Handler(void);

// SysTick register definitions
#define STCTRL *((volatile long *) 0xE000E010)
#define STRELOAD *((volatile long *) 0xE000E014)
#define STCURRENT *((volatile long *) 0xE000E018)

// Bit masks for SysTick CSR (Control and Status Register)
#define ENABLE (1<<0)
#define INT_EN (1<<1)
#define Clk_SRC (1<<2)
#define COUNT_FLAG (1<<16)

// Main function
void main(void)
{
    // Enable clock for GPIO Port E and UART Module 5
    SYSCTL_RCGCGPIO_R |= (1<<5);
    SYSCTL_RCGCUART_R |= (1<<5);
    SYSCTL_RCGCGPIO_R |= (1<<4);

    // Configure UART and Port F
    UART_Config();
    PortF_Config();

    while(1) {} // Endless loop
}

// Configure Port F
void PortF_Config(void)
{
    // Unlock and enable the PortF register for configuration
    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R = 0x1F;

    // Enable pull-up resistors for user switches, enable all pins on Port F
    GPIO_PORTF_PUR_R = 0x11;
    GPIO_PORTF_DEN_R = 0x1F;
    GPIO_PORTF_DIR_R = 0x0E; // Define PortF LEDs as output and switches as input

    // Configure PortF interrupts for user switches
    GPIO_PORTF_IS_R &= ~Sw_Bits; // Edge trigger detected
    GPIO_PORTF_IBE_R &= ~Sw_Bits; // Trigger interrupt according to GPIOIEV
    GPIO_PORTF_IEV_R &= ~Sw_Bits; // Trigger interrupt on falling edge
    GPIO_PORTF_IM_R &= ~Sw_Bits; // Mask interrupt bits
    GPIO_PORTF_ICR_R |= Sw_Bits; // Clear any prior interrupts
    GPIO_PORTF_IM_R |= Sw_Bits; // Enable interrupts for bits corresponding to Mask_Bits

    // NVIC Configuration for Port F interrupts
    // PortF interrupts correspond to interrupt 30 (EN0 and PRI7 registers)
    NVIC_EN0_R |= (1<<30); // Enable interrupts for Port F
    NVIC_PRI7_R &= 0xFF3FFFFF; // Set interrupt priority to 1 for Port F
}

// Configure UART Module 5
void UART_Config(void)
{
    // Calculate BRD (Baud Rate Divisor) for 9600 baud rate
    // UARTSysClk = 16MHz, ClkDiv = 16, Baud Rate = 9600
    // BRD = UARTSysClk / (ClkDiv * Baud Rate)
    // BRD = 104.167; BRDI = 104; BRDF = 167;
    // UARTFBRD[DIVFRAC] = integer(BRDF * 64 + 0.5) = 11

    // Disable UART module 5, set integer and fractional baud rate, set word length, FIFO, and parity enable
    UART5_CTL_R &= (0<<0);
    UART5_IBRD_R = 104;
    UART5_FBRD_R = 11;
    UART5_CC_R = 0x00; // System Clock
    UART5_LCRH_R = 0x62; // 8-bit word length, FIFO enable, Parity Enable
    UART5_CTL_R |= ((1<<0)|(1<<8)|(1<<9)); // Enable UART module 5

    // Configure UART interrupt
    UART5_IM_R &= ((0<<4)|(0<<5)|(0<<8)); // Mask Tx, Rx, and Parity interrupts
    UART5_ICR_R &= ((0<<4)|(0<<5)|(0<<8)); // Clear Tx, Rx, and Parity interrupts
    UART5_IM_R |= (1<<4); // Enable Rx interrupt
    NVIC_EN1_R |= (1<<29); // Enable interrupts for UART5
    NVIC_PRI15_R &= 0xFFFF5FFF; // Set interrupt priority to 2 for UART5

    // Unlock and enable the PortE register for configuration
    GPIO_PORTE_LOCK_R = 0x4C4F434B;
    GPIO_PORTE_CR_R = 0xFF;
    GPIO_PORTE_DEN_R = 0xFF; // Enable all pins on Port E
    GPIO_PORTE_DIR_R |= (1<<5); // Define PE5 as output
    GPIO_PORTE_AFSEL_R |= 0x30; // Enable Alternate function for PE4 and PE5
    GPIO_PORTE_PCTL_R |= 0x00110000; // Select UART function for PD6 and PD7
}

// Port F interrupt handler
void PortF_Handler()
{
    GPIO_PORTF_IM_R &= ~Sw_Bits; // Disable further Port F interrupts

    // Check which user switch triggered the interrupt
    if(GPIO_PORTF_RIS_R & 0x10) // User Switch 1
    {
        UART5_DR_R = 0xF0; // Transmit "F0"
    }
    else if (GPIO_PORTF_RIS_R & 0x01) // User Switch 2
    {
        UART5_DR_R = 0xAA; // Transmit "AA"
    }
}

// UART interrupt handler
void UART_Handler(void)
{
    UART5_IM_R &= (0<<4); // Mask UART Rx interrupt

    // Check if UART received data
    if(UART5_FR_R & (1<<6)) // Rx flag register set (data received)
    {
        if(UART5_DR_R == 0xAA)
        {
            GPIO_PORTF_DATA_R = Green; // Turn on LED in GREEN
        }
        else if(UART5_DR_R == 0xF0)
        {
            GPIO_PORTF_DATA_R = Blue; // Turn on LED in BLUE
        }
    }

    // Check for UART errors
    if(UART5_RSR_R & 0x0000000F) // Any error detected
    {
        GPIO_PORTF_DATA_R = Red; // Turn on LED in RED
    }

    UART5_ECR_R &= 0xFFFFFFF0; // Clear UART errors

    // Reinitialize the SysTick counter to zero
    STCURRENT = 0x00;
    // Set SysTick reload value to run SysTick for 0.5 seconds
    STRELOAD = 16*1000000/2;
    // Enable SysTick, enable interrupt generation, and use system clock (80MHz) as the source
    STCTRL |= (ENABLE | INT_EN | Clk_SRC);

    GPIO_PORTF_ICR_R = Sw_Bits; // Clear switch-related interrupts
}

// SysTick interrupt handler
void Systick_Handler(void)
{
    GPIO_PORTF_DATA_R &= 0x00; // Turn off the LED

    // Clear and unmask GPIO interrupts for switches
    GPIO_PORTF_ICR_R = Sw_Bits;
    GPIO_PORTF_IM_R |= Sw_Bits;

    // Unmask UART Rx interrupt
    UART5_IM_R |= (1<<4);
}
