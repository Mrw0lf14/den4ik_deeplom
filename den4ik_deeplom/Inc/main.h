#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f1xx.h"
#include "string.h"
#include "stdio.h"
#include <stdbool.h>

#define	RX_BUFF_SIZE	256
#define TX_BUFF_SIZE	256

/* Управление пинами */
#define	LED_ON()		GPIOA->BSRR = GPIO_BSRR_BS5
#define	LED_OFF()		GPIOA->BSRR = GPIO_BSRR_BR5
#define LED_SWAP()		GPIOA->ODR ^= GPIO_ODR_ODR5

#define	EN1_ON()		GPIOA->BSRR = GPIO_BSRR_BS8
#define	EN1_OFF()		GPIOA->BSRR = GPIO_BSRR_BR8
#define EN1_SWAP()		GPIOA->ODR ^= GPIO_ODR_ODR8

#define	EN2_ON()		GPIOA->BSRR = GPIO_BSRR_BS9
#define	EN2_OFF()		GPIOA->BSRR = GPIO_BSRR_BR9
#define EN2_SWAP()		GPIOA->ODR ^= GPIO_ODR_ODR9


#endif /* MAIN_H_ */
