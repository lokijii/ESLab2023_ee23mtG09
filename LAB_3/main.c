//LAB3- SYSTICK

/* SysTick memory-mapped registers */
#include<stdint.h>
#include<tm4c123gh6pm.h>

#define STCTRL *((volatile long *) 0xE000E010)    // control and status
#define STRELOAD *((volatile long *) 0xE000E014)    // reload value
#define STCURRENT *((volatile long *) 0xE000E018)    // current value

#define COUNT_FLAG  (1 << 16)   // bit 16 of CSR automatically set to 1
                                //   when timer expires
#define ENABLE      (1 << 0)    // bit 0 of CSR to enable the timer
#define CLKINT      (1 << 2)    // bit 2 of CSR to specify CPU clock

#define CLOCK_MHZ 16

void Delay(int us)
{
    STRELOAD = CLOCK_MHZ * us - 1; // reload value for 'us' microseconds
    STCTRL |= (CLKINT | ENABLE);        // set internal clock, enable the timer

    while ((STCTRL & COUNT_FLAG) == 0)  // wait until flag is set
    {
        ;   // do nothing
    }
    STCTRL = 0;                // stop the timer

    return;
}

int main(void)
{
    SYSCTL_RCGCGPIO_R |= 0x20; // enable clock to PORTF
        GPIO_PORTF_DIR_R |= 0x02; // set PF1 as output
        GPIO_PORTF_DEN_R |= 0x02; // enable digital function on PF1

        while (1)
        {
            GPIO_PORTF_DATA_R |= 0x02; // turn on PF1
            Delay(800); // delay for 800 microseconds
            GPIO_PORTF_DATA_R &= ~0x02; // turn off PF1
            Delay(200); // delay for 200 microseconds
        }
}

__________
