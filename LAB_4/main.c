//LAB4 - GPIO Interrupts and SysTick Timer Interrupt

Lines added to tm4c123gh6pm_startup_ccs.c:
Line 58 : void SysTickHandler (void) ;
Line 59 : void IntPortFHandler (void) ;
Line 87 : SysTickHandler, // The SysTick handler
Line 118: IntPortFHandler, / / GPIO Port F

#include<stdint.h>
#include<stdbool.h>
#include<tm4c123gh6pm.h>

//Control and Status Register
#define STCTRL *((volatile long *) 0xE000E010)    // Control and status register
#define STRELOAD *((volatile long *) 0xE000E014)    // SysTick Reload Value register
#define STCURRENT *((volatile long *) 0xE000E018)    // Systick Current Value Register

//Definitions to configure SysTick CSR (Control and Status Register)
#define ENABLE (1 << 0)           // bit 0 of CSR to enable the SysTick timer
#define INT_EN (1<<1)            //bit 1 of CSR to generate interrupt to the NVIC when Systick counts to 0
#define CLK_SRC (1<<2)    // bit 2 of CSR to specify CPU clock
#define COUNT_FLAG  (1<<16)   // bit 16 of CSR automatically set to 1 when timer expires
#define Mask_Bits 0x11

// Configure Port F ; Enable clock to Port F ;
// LEDs as digital output ,  Switches as digital input

void PortFConfig (void)
{
 SYSCTL_RCGC2_R |= 0x00000020; //enable clock to GPIOF
 GPIO_PORTF_LOCK_R = 0x4C4F434B; //Unlock PORTF register
 GPIO_PORTF_CR_R = 0x1F; // Enable Commit function
 GPIO_PORTF_DEN_R = 0x1F; //Enable all pins on PORTF
 GPIO_PORTF_DIR_R = 0x0E; //Set LEDs as outputs and switches as inputs
 GPIO_PORTF_PUR_R = 0x11; // Pull-up for user switches
}


void IntPortFHandler(void)
{
    GPIO_PORTF_DATA_R = 0x0E ;
    STRELOAD = 8000000; // 80 MHz clock counting for 80e6 will give 1 second
    STCTRL |= (ENABLE | INT_EN | CLK_SRC) ;
}

void SysTickHandler(void)
{
    GPIO_PORTF_DATA_R = 0x00 ;
    GPIO_PORTF_IM_R &= ~Mask_Bits;
    GPIO_PORTF_ICR_R = Mask_Bits ;
    GPIO_PORTF_IM_R |= Mask_Bits ;
}


int main(void)
{

    PortFConfig();

    //PortF Interrupt Configuration, User SW should trigger hardware interrupt
    GPIO_PORTF_IS_R &= ~Mask_Bits; //Edge trigger detected
    GPIO_PORTF_IBE_R &= ~Mask_Bits; //Trigger Interrupt according to GPIOIEV
    GPIO_PORTF_IEV_R &= ~Mask_Bits; //Trigger Interrupt on falling edge
    GPIO_PORTF_IM_R &= ~Mask_Bits; //Mask interrupt bits

    GPIO_PORTF_ICR_R |= Mask_Bits; //clear any prior interrupts
    GPIO_PORTF_IM_R |= Mask_Bits; //enable interrupts for bits corresponding to Mask_Bits

    //NVIC Configuration
    //PortF interrupts correspond to interrupt 30
    //(EN0 and PRI7 registers)
    NVIC_EN0_R |= (1<<30);   //Interrupts enabled for PortF
    NVIC_PRI7_R &= 0xFF3FFFFF; //Interrupt priority 1 to PortF
    while(1);

}
