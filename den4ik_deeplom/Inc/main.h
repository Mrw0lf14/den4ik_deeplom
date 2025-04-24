#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f1xx.h"
#include "string.h"
#include "stdio.h"
#include <stdbool.h>

#define	RX_BUFF_SIZE	256
#define TX_BUFF_SIZE	256

/* Управление пинами */
#define	LED_ON()		GPIOB->BSRR = GPIO_BSRR_BS12
#define	LED_OFF()		GPIOB->BSRR = GPIO_BSRR_BR12
#define LED_SWAP()		GPIOB->ODR ^= GPIO_ODR_ODR12

#define	EN1_ON()		GPIOB->BSRR = GPIO_BSRR_BS14
#define	EN1_OFF()		GPIOB->BSRR = GPIO_BSRR_BR14
#define EN1_SWAP()		GPIOB->ODR ^= GPIO_ODR_ODR14

#define	EN2_ON()		GPIOB->BSRR = GPIO_BSRR_BS13
#define	EN2_OFF()		GPIOB->BSRR = GPIO_BSRR_BR13
#define EN2_SWAP()		GPIOB->ODR ^= GPIO_ODR_ODR13


#endif /* MAIN_H_ */
