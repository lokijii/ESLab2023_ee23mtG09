#include <stdint.h>
#include "tm4c123gh6pm.h"

void PortF_Init(void);
void Delay(void);

int main(void) {
    PortF_Init(); // Initialize Port F

    uint8_t ledColor = 0; // 0 = red, 1 = green, 2 = blue

    // Turn on the red LED initially
    GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~0x0E) | 0x02;

    while (1) {
        // Check if SW1 (PF4) is pressed
        if ((GPIO_PORTF_DATA_R & 0x10) == 0) {
            ledColor = (ledColor + 1) % 3; // Cycle through red, green, blue
            switch (ledColor) {
                case 0: // Red
                    GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~0x0E) | 0x02;
                    break;
                case 1: // Green
                    GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~0x0E) | 0x08;
                    break;
                case 2: // Blue
                    GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~0x0E) | 0x04;
                    break;
            }
            Delay(); // Add a delay to avoid switch bouncing
        }
    }
}

void PortF_Init(void) {
    volatile uint32_t delay;
    SYSCTL_RCGCGPIO_R |= 0x20;   // Enable clock for Port F
//delay = SYSCTL_RCGCGPIO_R;   // Allow time for clock to stabilize

    GPIO_PORTF_LOCK_R = 0x4C4F434B; // Unlock the Port F
    GPIO_PORTF_CR_R |= 0x1F;        // Allow changes to PF4-0
    GPIO_PORTF_DIR_R &= ~0x10;      // PF4 is input (SW1)
    GPIO_PORTF_DIR_R |= 0x0E;       // PF1-PF3 are outputs (LEDs)
    GPIO_PORTF_AFSEL_R &= ~0x1F;    // Disable alternate functions
    GPIO_PORTF_PUR_R |= 0x10;       // Enable pull-up resistor for PF4 (SW1)
    GPIO_PORTF_DEN_R |= 0x1F;       // Digital enable for PF4-0
    GPIO_PORTF_AMSEL_R = 0;         // Disable analog functionality
}

void Delay(void) {
    volatile uint32_t delay = 1000000;
    while (delay > 0) {
        delay--;
    }
}
