#include <stdint.h>
#include "main.h"

void initClk(void)
{
	// Enable HSI
	RCC->CR |= RCC_CR_HSION;
	while(!(RCC->CR & RCC_CR_HSIRDY)){};
	// Enable Prefetch Buffer
	FLASH->ACR |= FLASH_ACR_PRFTBE;
	// Flash 2 wait state
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	FLASH->ACR |= FLASH_ACR_LATENCY_2;
	// HCLK = SYSCLK
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
	// PCLK2 = HCLK
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
	// PCLK1 = HCLK/2
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
	// PLL configuration: PLLCLK = HSI/2 * 16 = 64 MHz
	RCC->CFGR &= ~RCC_CFGR_PLLSRC;
	RCC->CFGR |= RCC_CFGR_PLLMULL16;
	// Enable PLL
	RCC->CR |= RCC_CR_PLLON;
	// Wait till PLL is ready
	while((RCC->CR & RCC_CR_PLLRDY) == 0) {};
	// Select PLL as system clock source
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	// Wait till PLL is used as system clock source
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL){};
}

void initTIM2(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;		//Включить тактирование TIM6

	//Частота APB1 для таймеров = APB1Clk * 2 = 32МГц * 2 = 64МГц
	TIM2->PSC = 64-1;					//Предделитель частоты (64МГц/64 = 1МГц)
	TIM2->ARR = 1000-1;						//Модуль счёта таймера (1МГц/1000 = 1мс)
	TIM2->DIER |= TIM_DIER_UIE;				//Разрешить прерывание по переполнению таймера
	TIM2->CR1 |= TIM_CR1_CEN;				//Включить таймер

	NVIC_EnableIRQ(TIM2_IRQn);				//Рарзрешить прерывание от TIM2
	NVIC_SetPriority(TIM2_IRQn, 1);			//Выставляем приоритет
}

void initTIM2_PWM(void)
{

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
	GPIOA->CRL |= GPIO_CRL_MODE0_0|GPIO_CRL_CNF0_1;	//PA0, альтернативный выход 2МГц

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;				//Включить тактирование TIM2

	//Частота APB1 для таймеров = APB1Clk * 2 = 32МГц * 2 = 64МГц
	TIM2->PSC = 100-1;								//Предделитель частоты (64000МГц/100 = 640кГц)
	TIM2->ARR = 320-1;								//Модуль счёта таймера (640кГц/320 = 2кГц)
	//TIM2->ARR = 80-1;								//Модуль счёта таймера (640кГц/80 = 8кГц)
	TIM2->CCR1 = TIM2->ARR/2;
	TIM2->DIER |= TIM_DIER_UIE;						//Разрешить прерывание по переполнению таймера
	TIM2->CCER |= TIM_CCER_CC1E;					//разблокируем выход
	TIM2->CCMR1 |= TIM_CCMR1_OC1M; 					//pwm mod1 110
	TIM2->CR1 |= TIM_CR1_CEN;						//Включить таймер

	NVIC_EnableIRQ(TIM2_IRQn);				//Рарзрешить прерывание от TIM2
	NVIC_SetPriority(TIM2_IRQn, 1);			//Выставляем приоритет
}

int main(void)
{
	initClk();
    /* Loop forever */
	for(;;);
}
