#ifndef __DRV_GPIO_CONFIG_H
#define __DRV_GPIO_CONFIG_H

/*
index, rcc, gpiox, pin

rcc:
RCC_GPIOA   0
RCC_GPIOB   1
RCC_GPIOC   2
RCC_GPIOD   3
RCC_GPIOE   4
RCC_GPIOF   5
RCC_GPIOG   6

index:
0: LED
 */
#define DRV_GPIO_TABLE                                                              \
    {                                                                               \
        {0, 5, GPIOF, GPIO_PIN_7}                                                   \
    }

/*
index, irqn, prio
 */
#define DRV_GPIO_IRQ_TABLE                                                          \
    {                                                                               \
        {-1, 0}                                                                     \
    }

#endif
