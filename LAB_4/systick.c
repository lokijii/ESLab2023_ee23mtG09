/* SysTick memory-mapped registers */
#define STCTRL *((volatile long *) 0xE000E010)    // control and status
#define STRELOAD *((volatile long *) 0xE000E014)    // reload value
#define STCURRENT *((volatile long *) 0xE000E018)    // current value

#define COUNT_FLAG  (1 << 16)   // bit 16 of CSR automatically set to 1
                                //   when timer expires
#define ENABLE      (1 << 0)    // bit 0 of CSR to enable the timer
#define CLKINT      (1 << 2)    // bit 2 of CSR to specify CPU clock

#define CLOCK_MHZ 16

void Delay(int ms)
{
    STRELOAD = 5;                       // reload value for 'ms' milliseconds
    STCTRL |= (CLKINT | ENABLE);        // set internal clock, enable the timer

    while ((STCTRL & COUNT_FLAG) == 0)  // wait until flag is set
    {
        ;   // do nothing
    }
    STCTRL = 0;                // stop the timer

    return;
}

