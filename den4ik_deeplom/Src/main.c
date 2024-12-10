#include <stdint.h>
#include "main.h"

char RxBuffer[RX_BUFF_SIZE];					//Буфер приёма USART
char TxBuffer[TX_BUFF_SIZE];					//Буфер передачи USART
bool ComReceived;

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

void setTIM3_period(uint16_t period)
{
	TIM3->ARR = period;
	TIM3->CCR1 = period/2;
}

void initUSART2(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;						//включить тактирование альтернативных ф-ций портов
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;					//включить тактирование UART2

	GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2);		//PA2 на выход
	GPIOA->CRL |= (GPIO_CRL_MODE2_1 | GPIO_CRL_CNF2_1);

	GPIOA->CRL &= ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3);		//PA3 - вход
	GPIOA->CRL |= GPIO_CRL_CNF3_0;

	/*****************************************
	Скорость передачи данных - 9600
	Частота шины APB1 - 32МГц

	1. USARTDIV = 32'000'000/(16*115200) = 17.36
	2. 17 = 0x11
	3. 16*0.4 = 6
	4. Итого 0x116
	*****************************************/
	USART2->BRR = 0x116;

	USART2->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
	USART2->CR1 |= USART_CR1_RXNEIE;						//разрешить прерывание по приему байта данных

	NVIC_EnableIRQ(USART2_IRQn);
}

void txStr(char *str)
{
	uint16_t i;
	strcat(str,"\r\n");									//добавляем символ конца строки
	for (i = 0; i < strlen(str); i++)
	{
		USART2->DR = str[i];								//передаём байт данных
		while ((USART2->SR & USART_SR_TC)==0) {};			//ждём окончания передачи
	}
}

void initTIM3_PWM(void)
{

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	GPIOA->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_MODE6);
	GPIOA->CRL |= GPIO_CRL_MODE6_0|GPIO_CRL_CNF6_1;	//PA6, альтернативный выход 2МГц

	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;				//Включить тактирование TIM3

	//Частота APB1 для таймеров = APB1Clk * 2 = 32МГц * 2 = 64МГц
	TIM3->PSC = 100-1;								//Предделитель частоты (64000МГц/100 = 640кГц)
	TIM3->ARR = 320-1;								//Модуль счёта таймера (640кГц/320 = 2кГц)
	//TIM2->ARR = 80-1;								//Модуль счёта таймера (640кГц/80 = 8кГц)
	TIM3->CCR1 = TIM3->ARR/2;
	TIM3->DIER |= TIM_DIER_UIE;						//Разрешить прерывание по переполнению таймера
	TIM3->CCER |= TIM_CCER_CC1E;					//разблокируем выход
	TIM3->CCMR1 |= TIM_CCMR1_OC1M; 					//pwm mod1 110
	TIM3->CR1 |= TIM_CR1_CEN;						//Включить таймер

	NVIC_EnableIRQ(TIM3_IRQn);				//Рарзрешить прерывание от TIM2
	NVIC_SetPriority(TIM3_IRQn, 1);			//Выставляем приоритет
}

void ExecuteCommand(void)
{
//	txStr(RxBuffer, false);
	memset(TxBuffer,0,sizeof(TxBuffer));					//Очистка буфера передачи

	/* Обработчик команд */
	if (strncmp(RxBuffer,"*IDN?",5) == 0)					//Это команда "*IDN?"
	{
		//Она самая, возвращаем строку идентификации
		txStr(TxBuffer);
	}
	else if (strncmp(RxBuffer,"START",5) == 0)				//Команда запуска таймера?
	{
		TIM2->CR1 |= TIM_CR1_CEN;
		strcpy(TxBuffer, "OK");
	}
	else if (strncmp(RxBuffer,"STOP",4) == 0)				//Команда остановки таймера?
	{
		TIM2->CR1 &= ~TIM_CR1_CEN;
		strcpy(TxBuffer, "OK");
	}
	else if (strncmp(RxBuffer,"PERIOD",6) == 0)				//Команда изменения периода таймера?
	{
		uint16_t tim_value;
		sscanf(RxBuffer,"%*s %hu", &tim_value);				//преобразуем строку в целое число

		if ((100 <= tim_value) && (tim_value <= 5000))		//параметр должен быть в заданных пределах!
		{
			TIM2->ARR = tim_value;
			TIM2->CNT = 0;

			strcpy(TxBuffer, "OK");
		}
		else
			strcpy(TxBuffer, "Parameter is out of range");	//ругаемся
	}
	else
		strcpy(TxBuffer,"Invalid Command");					//Если мы не знаем, чего от нас хотят, ругаемся в ответ

	// Передача принятой строки обратно одним из двух способов
	txStr(TxBuffer);

	memset(RxBuffer,0,RX_BUFF_SIZE);						//Очистка буфера приёма
	ComReceived = false;									//Сбрасываем флаг приёма строки
}

void initGPIO()
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;			//включить тактирование GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	//очистка полей
	GPIOA->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);
	GPIOA->CRL |= GPIO_CRL_MODE5_1;				//PA5, выход 2МГц

	GPIOA->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_MODE8);
	GPIOA->CRH |= GPIO_CRH_MODE8_1;				//PA8, выход 2МГц

	GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
	GPIOA->CRH |= GPIO_CRH_MODE9_1;				//PA9, выход 2МГц

	GPIOA->CRH &= ~(GPIO_CRH_MODE11 | GPIO_CRH_CNF11);
	GPIOA->CRH |= GPIO_CRH_CNF11_1;			//Вход с подтяжкой
	GPIOA->BSRR |= GPIO_BSRR_BS11;			//Подтяжка к Vdd

	GPIOA->CRH &= ~(GPIO_CRH_MODE14 | GPIO_CRH_CNF14);
	GPIOA->CRH |= GPIO_CRH_CNF14_1;			//Вход с подтяжкой

	GPIOA->CRH &= ~(GPIO_CRH_MODE15 | GPIO_CRH_CNF15);
	GPIOA->CRH |= GPIO_CRH_CNF15_1;			//Вход с подтяжкой
	/* Настройка самого прерывания */

	// Настройка альтернативных фукнций портов.
	// Настройки портов с 12 по 15 хранятся в регистре AFIO_EXTICR4.
	// Регистры объединены в массив AFIO->EXTICR, нумерация массива начинается с нулевого элемента.
	// Поэтому настройки AFIO_EXTICR4 хранятся в AFIO->EXTICR[3]
	AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI11_PC;
	AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI14_PC;
	AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI15_PC;

	EXTI->FTSR |= EXTI_FTSR_TR11 | EXTI_FTSR_TR14 | EXTI_FTSR_TR15;			//Прерывание по спаду импульса (при нажатии на кнопку)
	EXTI->IMR |= EXTI_IMR_MR11 | EXTI_IMR_MR14 | EXTI_IMR_MR15;				//Выставляем маску - EXTI13

	NVIC_EnableIRQ(EXTI15_10_IRQn);			//Разрешаем прерывание
	NVIC_SetPriority(EXTI15_10_IRQn, 0);	//Выставляем приоритет
}


int main(void)
{
	initClk();
	initTIM3_PWM();
	initUSART2();
	initGPIO();
    /* Loop forever */
	while(1)
	{
	}
}

void EXTI15_10_IRQHandler(void)
{
	if (EXTI->PR & EXTI_PR_PR11) 		// нас интересует EXTI14
	{
		EXTI->PR |= EXTI_PR_PR11;
	}
	if (EXTI->PR & EXTI_PR_PR14) 		// нас интересует EXTI14
	{
		EXTI->PR |= EXTI_PR_PR14;
	}
	if (EXTI->PR & EXTI_PR_PR15) 		// нас интересует EXTI15
	{
		EXTI->PR |= EXTI_PR_PR15;
	}
}

void USART2_IRQHandler(void)
{
	if ((USART2->SR & USART_SR_RXNE)!=0)		//Прерывание по приёму данных?
	{
		uint8_t pos = strlen(RxBuffer);			//Вычисляем позицию свободной ячейки

		RxBuffer[pos] = USART2->DR;				//Считываем содержимое регистра данных

		if ((RxBuffer[pos]== 0x0A) && (RxBuffer[pos-1]== 0x0D))							//Если это символ конца строки
		{
			ComReceived = true;					//- выставляем флаг приёма строки
			return;								//- и выходим
		}
	}
}

void TIM3_IRQHandler(void)
{
	TIM3->SR &= ~TIM_SR_UIF;			//Сброс флага переполнения
}
