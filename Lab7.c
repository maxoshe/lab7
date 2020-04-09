#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"

#define 	BLUE_MASK 		0x04
#define		RED_MASK			0x02

int global_counter = 0; //global variable for counter

void
PortFunctionInit(void)
{

	volatile uint32_t ui32Loop;   

	// Enable the clock of the GPIO port that is used for the on-board LED and switch.
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;

    //
    // Do a dummy read to insert a few cycles after enabling the peripheral.
    //
    ui32Loop = SYSCTL_RCGC2_R;

	// Unlock GPIO Port F
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   
	GPIO_PORTF_CR_R |= 0x17;           // allow changes to PF0,PF1,PF2,PF4

    // Set the direction of PF1,PF2 as output
    GPIO_PORTF_DIR_R |= 0x06;
	
	// Set the direction of PF0,PF4 as input by clearing the bit
    GPIO_PORTF_DIR_R &= ~0x11;
	
    // Enable both PF0,PF1,PF2,PF4 for digital function.
    GPIO_PORTF_DEN_R |= 0x17;
	
	//Enable pull-up on PF0,PF4
	GPIO_PORTF_PUR_R |= 0x11; 

}

//Globally enable interrupts 
void IntGlobalEnable(void)
{
    __asm("    cpsie   i\n");
}

void Interrupt_Init(void)
{
	NVIC_EN0_R |= 0x40000000;  		// enable interrupt 30 in NVIC (GPIOF)
	NVIC_PRI7_R &= 0x00E00000; 		// configure GPIOF interrupt priority as 0
	GPIO_PORTF_IM_R |= 0x11;   		// arm interrupt on PF0,PF4
	GPIO_PORTF_IS_R &= ~0x11;     	// PF0,PF4 is edge-sensitive
	GPIO_PORTF_IBE_R |= 0x11;   	// PF0,PF4 both edges trigger 
	IntGlobalEnable();        		// globally enable interrupt
}

//interrupt handler
void GPIOPortF_Handler(void)
{
// acknowledge flag for PF0,PF4
	GPIO_PORTF_ICR_R |= 0x11;      
	
	//SW1 is pressed
  	if((GPIO_PORTF_DATA_R&0x10)==0) 
	{
		if(global_counter != 3)		//if the counter is not at 3 increment by 1
		{
			global_counter = global_counter + 1;	
		}
		else						//if the counter is at 3 set to zero
		{
			global_counter = 0;
		}
		SysCtlDelay(1000000);		//delay to avoid bounce
	}
	
	//SW2 is pressed
  	if((GPIO_PORTF_DATA_R&0x01)==0) 
	{
		if(global_counter != 0)		//if the counter is not at 0 decrement counter
		{
			global_counter = global_counter - 1;
		}
		else						//if the counter is at 0 set to 3
		{
			global_counter = 3;
		}
		SysCtlDelay(1000000);		//delay to avoid bounce
	}
}

int main(void)
{
	
	//initialize the GPIO ports	
	PortFunctionInit();
	
	//configure the GPIOF interrupt
	Interrupt_Init();
	
    //
    // Loop forever.
    //
    while(1)
    {
		if(global_counter == 0) 	//all LED off for 0
		{
			GPIO_PORTF_DATA_R = 0x00;
		}
		if(global_counter == 1)		//RED on for 1
		{
			GPIO_PORTF_DATA_R = RED_MASK;
		}
		if(global_counter == 2)		//BLUE on for 2
		{
			GPIO_PORTF_DATA_R = BLUE_MASK;
		}
		if(global_counter == 3)		//RED and BLUE on for 3
		{
			GPIO_PORTF_DATA_R = BLUE_MASK+RED_MASK;
		}
    }
}
