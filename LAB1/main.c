#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"  // Include the TM4C123GH6PM header file

void delay(uint32_t count) {
    volatile uint32_t i;
    for (i = 0; i < count; i++) {}
}

int main(void) {
    // Set the system clock to 50 MHz (Assuming you've set up the clock system)

    // Enable the GPIO port F (assuming the LED is connected to PF1)
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    while (!(SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5)) {} // Wait for the GPIO port to be ready

    // Set the direction of PF1 as an output
    GPIO_PORTF_DIR_R |= 0x02;  // PF1 as output
    // Enable PF1 for digital function
    GPIO_PORTF_DEN_R |= 0x02;  // Digital enable PF1

    while (1) {
        // Toggle the LED (PF1) by XORing its current value
        GPIO_PORTF_DATA_R ^= 0x02; // Toggle PF1
        delay(1000000); // Delay for a short period to control blinking speed
    }

    return 0;
}
