// Description----------------------------------------------------------------|
/*
 * Initialises a struct with Name and Age data. Displays results on LEDs and
 * LCD.
 */
// DEFINES AND INCLUDES-------------------------------------------------------|

#define STM32F051
//>>> Uncomment line 10 if using System Workbench (SW4STM32) or STM32CubeIDE
#define SW4STM32

#ifndef SW4STM32
	#define TRUESTUDIO
#endif

#include "stm32f0xx.h"

// GLOBAL VARIABLES ----------------------------------------------------------|


// FUNCTION DECLARATIONS -----------------------------------------------------|

void main(void);                                                   //COMPULSORY
void init_ADC(void);											   //COMPULSORY
void init_timer_2(void);
void init_timer_6(void);

#ifdef TRUESTUDIO												   //COMPULSORY
	void reset_clock_to_48Mhz(void);							   //COMPULSORY
#endif															   //COMPULSORY



// MAIN FUNCTION -------------------------------------------------------------|

void main(void)
{
#ifdef TRUESTUDIO  											 	   //COMPULSORY
	reset_clock_to_48Mhz();										   //COMPULSORY
#endif															   //COMPULSORY

	init_timer_2();
	init_timer_6();
	NVIC_EnableIRQ(TIM6_DAC_IRQn);

	while(1)
	{

	}
}

// OTHER FUNCTIONS -----------------------------------------------------------|

#ifdef TRUESTUDIO												   //COMPULSORY
/* Description:
 * This function resets the STM32 Clocks to 48 MHz
 */
void reset_clock_to_48Mhz(void)									   //COMPULSORY
{																   //COMPULSORY
	if ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL)			   //COMPULSORY
	{															   //COMPULSORY
		RCC->CFGR &= (uint32_t) (~RCC_CFGR_SW);					   //COMPULSORY
		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);	   //COMPULSORY
	}															   //COMPULSORY

	RCC->CR &= (uint32_t)(~RCC_CR_PLLON);						   //COMPULSORY
	while ((RCC->CR & RCC_CR_PLLRDY) != 0);						   //COMPULSORY
	RCC->CFGR = ((RCC->CFGR & (~0x003C0000)) | 0x00280000);		   //COMPULSORY
	RCC->CR |= RCC_CR_PLLON;									   //COMPULSORY
	while ((RCC->CR & RCC_CR_PLLRDY) == 0);						   //COMPULSORY
	RCC->CFGR |= (uint32_t) (RCC_CFGR_SW_PLL);					   //COMPULSORY
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);		   //COMPULSORY
}																   //COMPULSORY
#endif															   //COMPULSORY

void init_ADC(void)
{
	//
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	//
	GPIOA->MODER |= GPIO_MODER_MODER6;
	//
	RCC->APB2ENR |= RCC_APB2ENR_ADCEN;
	//
	ADC1->CR |= ADC_CR_ADEN;
	//
	ADC1->CHSELR |= ADC_CHSELR_CHSEL6;
	// Setup in SINGLE SHOT mode
	ADC1->CFGR1 &= ~ADC_CFGR1_CONT;
	// Setup 10 bit resolution
	ADC1->CFGR1 |= ADC_CFGR1_RES_0;
	//
	while((ADC1->ISR & ADC_ISR_ADRDY) == 0);
}

void init_timer_2(void)		
{
	RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;	// Set clock to timer 2
	RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;	// Enable clock for Port B
	GPIOB -> MODER |= GPIO_MODER_MODER10_1;	// Set B10 to alternate function
	GPIOB -> AFR[1] |= 0x02 << (4*2);		// Set AFR to AF2 for B10 (TIM2_CH3)

	TIM2 -> PSC = 2;						// Set prescaler to 2 to math ARR with CCR
	TIM2 -> ARR = 1023;						// Set ARR to 1023 to achieve PWM signal of 15kHz
	TIM2 -> CCMR1 |= TIM_CCMR2_OC3M_2
				  | TIM_CCMR2_OC3M_1 	    // Configure PWM mode on OC3 (OC3 because AF2 for B10 is TIM2_CH3!)
				  | TIM_CCMR2_OC3PE; 	    // Enable preload register on OC3
	TIM2 -> CCER |= TIM_CCER_CC3E;			// Enable output on OC3
	TIM2 -> CR1 |= TIM_CR1_CEN;				// Enable counter
	
	TIM2 -> CCR3 = 1023;					// Configure duty cycle for 100% of ARR (using CCR3 because TIM2_CH3)
}

void init_timer_6(void)
{
	RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;	// Set clock to timer 6
	TIM6 -> PSC = 36;						// PSC = 36 to generate delay of 50mS
	TIM6 -> ARR = 66666;					// ARR = 66 666 to generate delay of 50mS

	TIM6 -> DIER |= TIM_DIER_UIE;			// Allow overflow interrupts
	TIM6 -> CR1 |= TIM_CR1_CEN;				// Enable counter for timer 6
	NVIC_EnableIRQ(TIM6_DAC_IRQn);			// Enable interrupt
}

// INTERRUPT HANDLERS --------------------------------------------------------|

void TIM6_DAC_IRQHandler(void)
{				
	TIM6->SR &= ~TIM_SR_UIF;				// Acknowledge interrupt
	init_ADC();
	init_timer_2();
}
