// Copyright © 2023 Vouch.io LLC

#define DT_DRV_COMPAT st_st25r

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

#include "platform.h"

#include "st25r.h"

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(spi)

LOG_MODULE_DECLARE(ST25R);

static const struct device *s_spi_dev;
static const struct gpio_dt_spec *s_cs_dev;

int st25r_spi_init(const struct device *dev)
{
    const struct st25r_device_config *config = dev->config;

    if (!spi_is_ready_dt(&config->spi)) {
        LOG_ERR("Bus device is not ready");
        return -ENODEV;
    }

    s_cs_dev = &config->spi.config.cs.gpio;
    s_spi_dev = dev;

    return 0;
}

static void cs_assert(int dir)
{
    if (s_cs_dev && gpio_is_ready_dt(s_cs_dev)) {
        gpio_pin_set_dt(s_cs_dev, dir);
    } else {
        LOG_ERR("Unable to access CS");
    }
}

void platform_st25r_spi_select()
{
    LOG_DBG("SPI select");
    cs_assert(1);
}

void platform_st25r_spi_deselect()
{
    LOG_DBG("SPI deselect");
    cs_assert(0);
}

void platform_st25r_spi_transceive(const uint8_t *txBuf, uint8_t *rxBuf, uint16_t len)
{
    LOG_DBG("platform_st25r_spi_transceive %d bytes, txBuf: %p rxBuf: %p", len, txBuf, rxBuf);

    const struct st25r_device_config *config = s_spi_dev->config;
    const struct spi_buf tx_buf[1] = {
            {
                    .buf = (uint8_t *) txBuf,
                    .len = len,
            },
    };
    const struct spi_buf_set tx = {
            .buffers = tx_buf,
            .count = ARRAY_SIZE(tx_buf),
    };
    const struct spi_buf rx_buf[1] = {
            {
                    .buf = rxBuf,
                    .len = len,
            },
    };
    const struct spi_buf_set rx = {
            .buffers = rx_buf,
            .count = ARRAY_SIZE(rx_buf),
    };

    if (spi_transceive_dt(&config->spi, &tx, &rx)) {
        LOG_ERR("error with spi transceive");
    }

}

#endif /* DT_ANY_INST_ON_BUS_STATUS_OKAY(spi) */
