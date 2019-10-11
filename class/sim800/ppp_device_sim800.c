/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2019-08-15    xiangxistu      the first version
 */

#include <ppp_device_sim800.h>
#include <ppp_chat.h>
#include <rtdevice.h>

#define DBG_TAG    "ppp.sim800"

#ifdef PPP_DEVICE_DEBUG
#define DBG_LVL   DBG_LOG
#else
#define DBG_LVL   DBG_INFO
#endif
#include <rtdbg.h>

#define SIM800_POWER_ON  PIN_HIGH
#define SIM800_POWER_OFF PIN_LOW
#ifndef SIM800_POWER_PIN
#define SIM800_POWER_PIN -1
#endif

static const struct modem_chat_data rst_mcd[] =
{
    {"+++",          MODEM_CHAT_RESP_NOT_NEED,        30, 1, RT_TRUE},
    {"ATH",          MODEM_CHAT_RESP_OK,              30, 1, RT_FALSE},
};

static const struct modem_chat_data mcd[] =
{
    {"AT",           MODEM_CHAT_RESP_OK,              10, 1, RT_FALSE},
    {"ATE0",         MODEM_CHAT_RESP_OK,              1,  1, RT_FALSE},
    {PPP_APN_CMD,    MODEM_CHAT_RESP_OK,              1,  5, RT_FALSE},
    {PPP_DAIL_CMD,   MODEM_CHAT_RESP_CONNECT,         1, 30, RT_FALSE},
};

static rt_err_t ppp_sim800_prepare(struct ppp_device *device)
{
    struct ppp_sim800 *sim800 = (struct ppp_sim800*)device;
    if (sim800->power_pin >= 0)
    {
        rt_pin_write(sim800->power_pin, SIM800_POWER_OFF);
        rt_thread_mdelay(SIM800_WARTING_TIME_BASE);
        rt_pin_write(sim800->power_pin, SIM800_POWER_ON);
    }
    else
    {
        rt_err_t err;
        err = modem_chat(device->uart, rst_mcd, sizeof(rst_mcd) / sizeof(rst_mcd[0]));
        if (err)
            return err;
    }
    return modem_chat(device->uart, mcd, sizeof(mcd) / sizeof(mcd[0]));
}

/* ppp_sim800_ops for ppp_device_ops , a common interface */
static struct ppp_device_ops sim800_ops =
{
    .prepare = ppp_sim800_prepare,
};

/*
 * register sim800 into ppp_device
 *
 * @param
 *
 *
 * @return  ppp_device function piont
 *
 */
int ppp_sim800_register(void)
{
    struct ppp_device *ppp_device = RT_NULL;
    struct ppp_sim800 *sim800 = RT_NULL;

    sim800 = rt_malloc(sizeof(struct ppp_sim800));
    if(sim800 == RT_NULL)
    {
        LOG_E("No memory for struct sim800.");
        return -RT_ENOMEM;
    }

    sim800->power_pin = SIM800_POWER_PIN;
    if (sim800->power_pin >= 0)
    {
        rt_pin_mode(sim800->power_pin, PIN_MODE_OUTPUT);
        rt_pin_write(sim800->power_pin, SIM800_POWER_OFF);
    }

    ppp_device = &(sim800->device);
    ppp_device->ops = &sim800_ops;

    LOG_D("ppp sim800 is registering ppp_device");

    return ppp_device_register(ppp_device, PPP_DEVICE_NAME, RT_NULL, RT_NULL);
}
INIT_COMPONENT_EXPORT(ppp_sim800_register);
