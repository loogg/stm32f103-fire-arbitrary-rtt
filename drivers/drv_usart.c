#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "stm32f1xx_hal.h"
#include "drv_usart_config.h"

#ifdef BSP_USING_UART

struct stm32_usart_config
{
    const char *name;
    UART_HandleTypeDef *handle;
    USART_TypeDef *instance;
    DMA_HandleTypeDef *dma_tx;
    DMA_HandleTypeDef *dma_rx;
    uint8_t only_sync;
    rt_uint8_t *read_buf;
    int read_bufsz;
    int rs485_control_pin;
    int rs485_send_logic;
};



#endif /* BSP_USING_UART */
