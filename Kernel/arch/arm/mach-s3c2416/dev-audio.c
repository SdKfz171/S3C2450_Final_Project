/* linux/arch/arm/plat-s3c/dev-audio.c
 *
 * Copyright 2009 Wolfson Microelectronics
 *      Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>

#include <mach/irqs.h>
#include <mach/map.h>
#include <mach/dma.h>

#include <plat/devs.h>
#include <plat/audio.h>
#include <plat/gpio-cfg.h>

static const char *rclksrc[] = {
	[0] = "iis",
	[1] = "i2s-if",
};

static int s3c2416_i2s_cfg_gpio(struct platform_device *pdev)
{
        /* Configure the I2S pins in correct mode */
//      s3c_gpio_cfgpin_range(S3C2410_GPE(0), 5, S3C_GPIO_SFN(2));
#if 1
        s3c2410_gpio_cfgpin(S3C2410_GPE(0), S3C_GPIO_SFN(2)); //2
        s3c2410_gpio_cfgpin(S3C2410_GPE(1), S3C_GPIO_SFN(2)); //2
        s3c2410_gpio_cfgpin(S3C2410_GPE(2), S3C_GPIO_SFN(2)); //2
        s3c2410_gpio_cfgpin(S3C2410_GPE(3), S3C_GPIO_SFN(2)); //2
        s3c2410_gpio_cfgpin(S3C2410_GPE(4), S3C_GPIO_SFN(2)); //2
#endif
        return 0;
}

static struct resource s3c2416_iis_resource[] = {
        [0] = {
                .start = S3C2410_PA_IIS,
                .end   = S3C2410_PA_IIS + 0x100 - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = DMACH_I2S_OUT,
                .end   = DMACH_I2S_OUT,
                .flags = IORESOURCE_DMA,
        },
        [2] = {
                .start = DMACH_I2S_IN,
                .end   = DMACH_I2S_IN,
                .flags = IORESOURCE_DMA,
        },
};

static struct s3c_audio_pdata i2s_pdata = {
        .cfg_gpio = s3c2416_i2s_cfg_gpio,
        .type = {
                .i2s = {
                        .quirks = QUIRK_PRI_6CHAN | QUIRK_NO_DEFAULTGATE,
                        .src_clk = rclksrc,
                },
        },
};

struct platform_device s3c2416_device_iis = {
        .name             = "samsung-i2s",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3c2416_iis_resource),
        .resource         = s3c2416_iis_resource,
        .dev = {
                .platform_data = &i2s_pdata,
        },
};
EXPORT_SYMBOL(s3c2416_device_iis);
