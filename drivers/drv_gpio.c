#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "stm32f1xx_hal.h"
#include "drv_gpio_config.h"

#ifdef RT_USING_PIN

#define RCC_GPIOA   0
#define RCC_GPIOB   1
#define RCC_GPIOC   2
#define RCC_GPIOD   3
#define RCC_GPIOE   4
#define RCC_GPIOF   5
#define RCC_GPIOG   6

struct stm32_gpio
{
    int index;
    uint8_t rcc_gpiox;
    GPIO_TypeDef *gpiox;
    uint32_t pin;
};

struct stm32_gpio_irq
{
    int index;
    IRQn_Type irqno;
    uint8_t prio;
    uint8_t used;
    int mode;
    void (*hdr)(void *args);
    void *args;
};

static const struct stm32_gpio gpio_obj[] = DRV_GPIO_TABLE;

static struct stm32_gpio_irq gpio_irq_obj[] = DRV_GPIO_IRQ_TABLE;

static uint32_t pin_irq_enable_mask = 0;

static const struct stm32_gpio *get_gpio(int pin)
{
    int num = sizeof(gpio_obj) / sizeof(gpio_obj[0]);
    if((pin < 0) || (pin >= num))
        return RT_NULL;

    return &gpio_obj[pin];
}

static void stm32_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    const struct stm32_gpio *gpio = get_gpio(pin);
    if(gpio == RT_NULL)
        return;

    switch(gpio->rcc_gpiox)
    {
        case RCC_GPIOA:
            __HAL_RCC_GPIOA_CLK_ENABLE();
        break;

        case RCC_GPIOB:
            __HAL_RCC_GPIOB_CLK_ENABLE();
        break;

        case RCC_GPIOC:
            __HAL_RCC_GPIOC_CLK_ENABLE();
        break;

        case RCC_GPIOD:
            __HAL_RCC_GPIOD_CLK_ENABLE();
        break;

        case RCC_GPIOE:
            __HAL_RCC_GPIOE_CLK_ENABLE();
        break;

        case RCC_GPIOF:
            __HAL_RCC_GPIOF_CLK_ENABLE();
        break;

        case RCC_GPIOG:
            __HAL_RCC_GPIOG_CLK_ENABLE();
        break;

        default:
            return;
    }

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = gpio->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    switch(mode)
    {
        case PIN_MODE_OUTPUT:
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;

        case PIN_MODE_INPUT:
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;

        case PIN_MODE_INPUT_PULLUP:
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
        break;

        case PIN_MODE_INPUT_PULLDOWN:
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        break;

        case PIN_MODE_OUTPUT_OD:
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;

        default:
            return;
    }

    HAL_GPIO_Init(gpio->gpiox, &GPIO_InitStruct);
}

static void stm32_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
    const struct stm32_gpio *gpio = get_gpio(pin);
    if(gpio == RT_NULL)
        return;

    HAL_GPIO_WritePin(gpio->gpiox, gpio->pin, (GPIO_PinState)(value ? 1 : 0));
}

static int stm32_pin_read(rt_device_t dev, rt_base_t pin)
{
    int value = PIN_LOW;
    const struct stm32_gpio *gpio = get_gpio(pin);
    if(gpio == RT_NULL)
        return value;

    value = HAL_GPIO_ReadPin(gpio->gpiox, gpio->pin);

    return value;
}

static struct stm32_gpio_irq *get_gpio_irq(rt_int32_t pin)
{
    int num = sizeof(gpio_irq_obj) / sizeof(gpio_irq_obj[0]);
    for (int i = 0; i < num; i++)
    {
        if(gpio_irq_obj[i].index == pin)
            return &gpio_irq_obj[i];
    }

    return RT_NULL;
}

static rt_err_t stm32_pin_attach_irq(struct rt_device *device, rt_int32_t pin,
                                     rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    if(get_gpio(pin) == RT_NULL)
        return -RT_ERROR;

    struct stm32_gpio_irq *gpio_irq = get_gpio_irq(pin);
    if(gpio_irq == RT_NULL)
        return -RT_ERROR;

    rt_base_t level = rt_hw_interrupt_disable();
    if(gpio_irq->used)
    {
        if(mode != gpio_irq->mode)
        {
            rt_hw_interrupt_enable(level);
            return -RT_ERROR;
        }
        gpio_irq->hdr = hdr;
        gpio_irq->args = args;
    }
    else
    {
        gpio_irq->mode = mode;
        gpio_irq->hdr = hdr;
        gpio_irq->args = args;
    }
    rt_hw_interrupt_enable(level);

    return RT_EOK;
}

static rt_err_t stm32_pin_detach_irq(struct rt_device *device, rt_int32_t pin)
{
    if(get_gpio(pin) == RT_NULL)
        return -RT_ERROR;

    struct stm32_gpio_irq *gpio_irq = get_gpio_irq(pin);
    if(gpio_irq == RT_NULL)
        return -RT_ERROR;

    rt_base_t level = rt_hw_interrupt_disable();
    gpio_irq->hdr = RT_NULL;
    gpio_irq->args = RT_NULL;
    rt_hw_interrupt_enable(level);

    return RT_EOK;
}

static rt_err_t stm32_pin_irq_enable(struct rt_device *device, rt_base_t pin, rt_uint32_t enabled)
{
    const struct stm32_gpio *gpio = get_gpio(pin);
    if(gpio == RT_NULL)
        return -RT_ERROR;

    struct stm32_gpio_irq *gpio_irq = get_gpio_irq(pin);
    if(gpio_irq == RT_NULL)
        return -RT_ERROR;

    if(enabled == PIN_IRQ_ENABLE)
    {
        rt_base_t level = rt_hw_interrupt_disable();

        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = gpio->pin;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        switch(gpio_irq->mode)
        {
            case PIN_IRQ_MODE_RISING:
                GPIO_InitStruct.Pull = GPIO_PULLDOWN;
                GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
            break;

            case PIN_IRQ_MODE_FALLING:
                GPIO_InitStruct.Pull = GPIO_PULLUP;
                GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
            break;

            case PIN_IRQ_MODE_RISING_FALLING:
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
            break;

            default:
                rt_hw_interrupt_enable(level);
                return -RT_ERROR;
        }

        HAL_GPIO_Init(gpio->gpiox, &GPIO_InitStruct);
        __HAL_GPIO_EXTI_CLEAR_IT(gpio->pin);

        HAL_NVIC_SetPriority(gpio_irq->irqno, gpio_irq->prio, 0);
        HAL_NVIC_EnableIRQ(gpio_irq->irqno);

        gpio_irq->used = 1;
        pin_irq_enable_mask |= gpio->pin;
        rt_hw_interrupt_enable(level);
    }
    else if(enabled == PIN_IRQ_DISABLE)
    {
        rt_base_t level = rt_hw_interrupt_disable();
        HAL_GPIO_DeInit(gpio->gpiox, gpio->pin);
        gpio_irq->used = 0;
        pin_irq_enable_mask &= ~gpio->pin;
        if((gpio->pin >= GPIO_PIN_5) && (gpio->pin <= GPIO_PIN_9))
        {
            if(!(pin_irq_enable_mask & (GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9)))
            {
                HAL_NVIC_DisableIRQ(gpio_irq->irqno);
            }
        }
        else if((gpio->pin >= GPIO_PIN_10) && (gpio->pin <= GPIO_PIN_15))
        {
            if(!(pin_irq_enable_mask & (GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)))
            {
                HAL_NVIC_DisableIRQ(gpio_irq->irqno);
            }
        }
        else
        {
            HAL_NVIC_DisableIRQ(gpio_irq->irqno);
        }
        rt_hw_interrupt_enable(level);
    }
    else
        return -RT_ERROR;

    return RT_EOK;
}

static const struct rt_pin_ops stm32_pin_ops =
{
    stm32_pin_mode,
    stm32_pin_write,
    stm32_pin_read,
    stm32_pin_attach_irq,
    stm32_pin_detach_irq,
    stm32_pin_irq_enable,
    RT_NULL
};

void STM32_GPIO_IRQHandler(int index)
{
    int num = sizeof(gpio_irq_obj) / sizeof(gpio_irq_obj[0]);
    if((index < 0) || (index >= num))
        return;

    struct stm32_gpio_irq *gpio_irq = &gpio_irq_obj[index];

    if (gpio_irq->hdr)
        gpio_irq->hdr(gpio_irq->args);
}

int rt_hw_pin_init(void)
{
    return rt_device_pin_register("pin", &stm32_pin_ops, RT_NULL);
}

#endif /* RT_USING_PIN */
