#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"


// Define SysTick memory-mapped registers
#define STCTRL *((volatile long *) 0xE000E010)    // SysTick control and status register
#define STRELOAD *((volatile long *) 0xE000E014)  // SysTick reload value register
#define STCURRENT *((volatile long *) 0xE000E018) // SysTick current value register

#define COUNT_FLAG (1 << 16)  // Bit 16 of the Control and Status Register (CSR), set to 1 when the timer expires
#define ENABLE (1 << 0)      // Bit 0 of STCTRL to enable the timer
#define CLKINT (1 << 2)      // Bit 2 of STCTRL to specify the CPU clock

#define MASK_BITS 0x01  // Bit mask for user switch 2

void GPIOF_config(void);
void GPIOF_Interrupt_config(void);

void main(void)
{
    SYSCTL_RCC_R &= ~(1 << 20);      // Disable the system clock as the source for PWM (16MHz)
    SYSCTL_RCGC0_R |= 0x00100000;    // Enable the clock for the PWM module
    SYSCTL_RCGCPWM_R = 0x3;         // Enable clock for PWM modules 1

    GPIOF_config();                 // Configure GPIO pins
    GPIOF_Interrupt_config();       // Configure GPIO interrupts
    GPIO_PORTF_AFSEL_R |= 0x02;     // Enable alternate function on PF1
    GPIO_PORTF_PCTL_R |= 0x50;      // Set the M1PWM5 function on PF1

    PWM1_2_CTL_R = 0x00000000;      // Disable PWM module 1, generator 2
    PWM1_2_GENB_R |= 0x80E;         // Configure PWM module 1, generator 2, signal B: invert on compare B down count, high on load, low on zero
    PWM1_2_LOAD_R = 160;            // Set the load value for a 100kHz signal
    PWM1_2_CMPB_R = 80;            // Set the compare value for a 50% duty cycle
    PWM1_2_CTL_R |= 0x01;           // Enable the PWM block
    PWM1_ENABLE_R |= (1 << 5);     // Enable M1PWM5 signal on the pin

    while (1);
}

void GPIOF_config()
{
    SYSCTL_RCGC2_R |= 0x20;         // Enable the clock for port F
    GPIO_PORTF_LOCK_R = 0x4C4F434B; // Unlock the commit register
    GPIO_PORTF_CR_R = 0x1F;         // Make PORTF0 configurable
    GPIO_PORTF_DEN_R = 0x1F;        // Enable digital I/O on all PF pins
    GPIO_PORTF_DIR_R = 0x0E;        // Set PF0 and PF4 as input, PF1-PF3 as output
    GPIO_PORTF_PUR_R = 0x11;        // Enable pull-up resistor on PF0 and PF4
}

void GPIOF_Interrupt_config()
{
    GPIO_PORTF_IM_R &= ~MASK_BITS;    // Mask interrupts for SW2
    GPIO_PORTF_IS_R &= ~MASK_BITS;    // Configure edge-sensitive interrupts
    GPIO_PORTF_IBE_R |= MASK_BITS;    // Interrupt control is handled by GPIOIEV
    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF1FFFFF) | (2 << 21); // Set priority of interrupt 30 (Port F) to 2
    NVIC_EN0_R |= (1 << 30);          // Enable interrupt on Port F
    GPIO_PORTF_ICR_R = MASK_BITS;     // Clear Raw Interrupt Status (RIS) and Masked Interrupt Status (MIS) for edge-sensitive interrupt
    GPIO_PORTF_IM_R |= MASK_BITS;     // Unmask interrupts for SW2
}

void GPIOF_INT_Handler(void)
{
    GPIO_PORTF_IM_R &= ~MASK_BITS;  // Mask interrupts for SW2
    int i;
    if (GPIO_PORTF_RIS_R & MASK_BITS)  // Check if the interrupt is caused by SW2
    {
        for (i = 0; i < 1600; i++) {} // Implement a delay for debouncing
        if (~(GPIO_PORTF_DATA_R) & 0x01)  // If the input on SW2 is 0 (pressed), initiate a timer for debouncing
        {
            STCURRENT = 0;
            STRELOAD = 1000000 * 8;     // Set the reload value for 0.5 seconds
            STCTRL |= (CLKINT | ENABLE);  // Set internal clock and enable the timer
        }
        else
        {
            if (STCTRL & COUNT_FLAG) // More than half-second press
            {
                STCTRL = 0;  // Stop the timer
                if (PWM1_2_CMPB_R < 152)
                {
                    PWM1_2_CMPB_R += 8; // decrease duty cycle by 5%
                }
            }
            else // Short press
            {
                STCTRL = 0;  // Stop the timer
                if (PWM1_2_CMPB_R > 8)
                {
                    PWM1_2_CMPB_R -= 8; // decrease duty cycle by 5%
                }
            }
        }
    }
    GPIO_PORTF_ICR_R = MASK_BITS;  // Clear the interrupt for Port F
    GPIO_PORTF_IM_R |= MASK_BITS;  // Unmask interrupts for SW2
}


