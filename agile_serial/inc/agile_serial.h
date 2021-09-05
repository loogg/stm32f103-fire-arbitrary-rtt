#ifndef __PKG_AGILE_SERIAL_H
#define __PKG_AGILE_SERIAL_H
#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RT_USING_SERIAL
#error "Please close rtt Serial device drivers"
#endif

#define AGILE_SERIAL_DATA_BITS_5                     5
#define AGILE_SERIAL_DATA_BITS_6                     6
#define AGILE_SERIAL_DATA_BITS_7                     7
#define AGILE_SERIAL_DATA_BITS_8                     8
#define AGILE_SERIAL_DATA_BITS_9                     9

#define AGILE_SERIAL_STOP_BITS_1                     0
#define AGILE_SERIAL_STOP_BITS_2                     1
#define AGILE_SERIAL_STOP_BITS_3                     2
#define AGILE_SERIAL_STOP_BITS_4                     3

#define AGILE_SERIAL_PARITY_NONE                     0
#define AGILE_SERIAL_PARITY_ODD                      1
#define AGILE_SERIAL_PARITY_EVEN                     2

#define AGILE_SERIAL_BIT_ORDER_LSB                   0
#define AGILE_SERIAL_BIT_ORDER_MSB                   1

#define AGILE_SERIAL_ERROR_TX_TIMEOUT                0x001
#define AGILE_SERIAL_ERROR_RX_FULL                   0x002
#define AGILE_SERIAL_ERROR_OVERRUN                   0x004
#define AGILE_SERIAL_ERROR_FRAMING                   0x008
#define AGILE_SERIAL_ERROR_PARITY                    0x010
#define AGILE_SERIAL_ERROR_OTHER                     0x800

#define AGILE_SERIAL_CTRL_RESET                      0x05
#define AGILE_SERIAL_CTRL_FLUSH                      0x06
#define AGILE_SERIAL_CTRL_ASYNC                      0x07

#define AGILE_SERIAL_FLAG_ASYNC                      0x080

/* Default config for serial_configure structure */
#define AGILE_SERIAL_CONFIG_DEFAULT              \
{                                                \
    115200,           /* 115200 bits/s */        \
    AGILE_SERIAL_DATA_BITS_8,      /* 8 databits */     \
    AGILE_SERIAL_STOP_BITS_1,      /* 1 stopbit */      \
    AGILE_SERIAL_PARITY_NONE,      /* No parity  */     \
    AGILE_SERIAL_BIT_ORDER_LSB,    /* LSB first sent */ \
    0                                            \
}

struct agile_serial_configure
{
    rt_uint32_t baud_rate;

    rt_uint32_t data_bits               :4;
    rt_uint32_t stop_bits               :2;
    rt_uint32_t parity                  :2;
    rt_uint32_t bit_order               :1;
    rt_uint32_t reserved                :23;
};

struct agile_serial_device
{
    struct rt_device parent;

    rt_uint8_t sync_flag;
    rt_uint16_t error_flag;
    struct agile_serial_configure config;
    struct rt_ringbuffer rx_rb;
    const struct agile_serial_ops *ops;
    rt_err_t (*rx_indicate)(struct agile_serial_device *serial, rt_size_t size);
    rt_err_t (*tx_complete)(struct agile_serial_device *serial, void *buffer);
    rt_err_t (*error_indicate)(struct agile_serial_device *serial, rt_uint16_t error_flag);
};

struct agile_serial_ops
{
    rt_err_t (*init)(struct agile_serial_device *serial, struct serial_configure *cfg);
    rt_err_t (*deinit)(struct agile_serial_device *serial);
    rt_err_t (*stop)(struct agile_serial_device *serial);
    rt_err_t (*control)(struct agile_serial_device *serial, int cmd, void *arg);

    rt_size_t (*start_tx)(struct agile_serial_device *serial, const void *buffer, rt_size_t size);
    rt_err_t (*stop_tx)(struct agile_serial_device *serial);

    rt_err_t (*restart_rx)(struct agile_serial_device *serial);
};

#ifdef __cplusplus
}
#endif

#endif
