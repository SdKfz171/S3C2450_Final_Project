/* linux/arch/arm/mach-s3c2410/dma.c
 *
 * Copyright (c) 2006 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C2443 DMA selection
 *
 * http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysdev.h>
#include <linux/serial_core.h>
#include <linux/io.h>

#include <mach/map.h>
#include <mach/dma.h>

#include <plat/cpu.h>
#include <plat/dma-s3c24xx.h>

#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>
#include <plat/regs-ac97.h>
#include <plat/regs-dma.h>
#include <mach/regs-mem.h>
#include <mach/regs-lcd.h>
#include <mach/regs-sdi.h>
#include <plat/regs-iis.h>
#include <plat/regs-spi.h>

#define MAP(x) { \
                [0]     = (x) | DMA_CH_VALID,   \
                [1]     = (x) | DMA_CH_VALID,   \
                [2]     = (x) | DMA_CH_VALID,   \
                [3]     = (x) | DMA_CH_VALID,   \
                [4]     = (x) | DMA_CH_VALID,   \
                [5]     = (x) | DMA_CH_VALID,   \
        }

static struct s3c24xx_dma_map __initdata s3c2443_dma_mappings[] = {
	[DMACH_XD0] = {
		.name		= "xdreq0",
                .channels       = MAP(S3C2443_DMAREQSEL_XDREQ0),
	},
        [DMACH_XD1] = {
                .name           = "xdreq1",
                .channels       = MAP(S3C2443_DMAREQSEL_XDREQ1),
        },
        [DMACH_SDI] = { /* only on S3C2443 */
                .name           = "sdi",
                .channels       = MAP(S3C2443_DMAREQSEL_SDI),
        },
        [DMACH_SPI0_RX] = {
                .name           = "spi0-rx",
                .channels       = MAP(S3C2443_DMAREQSEL_SPI0RX),
        },
        [DMACH_SPI0_TX] = {
                .name           = "spi0-tx",
                .channels       = MAP(S3C2443_DMAREQSEL_SPI0TX),
        },
        [DMACH_SPI1_RX] = { /* only on S3C2443/S3C2450 */
                .name           = "spi1-rx",
                .channels       = MAP(S3C2443_DMAREQSEL_SPI1RX),
        },
        [DMACH_SPI1_TX] = { /* only on S3C2443/S3C2450 */
                .name           = "spi1-tx",
                .channels       = MAP(S3C2443_DMAREQSEL_SPI1TX),
        },
        [DMACH_UART0] = {
                .name           = "uart0",
                .channels       = MAP(S3C2443_DMAREQSEL_UART0_0),
        },
        [DMACH_UART1] = {
                .name           = "uart1",
                .channels       = MAP(S3C2443_DMAREQSEL_UART1_0),
        },
        [DMACH_UART2] = {
                .name           = "uart2",
                .channels       = MAP(S3C2443_DMAREQSEL_UART2_0),
        },
        [DMACH_UART3] = {
                .name           = "uart3",
                .channels       = MAP(S3C2443_DMAREQSEL_UART3_0),
        },
        [DMACH_UART0_SRC2] = {
                .name           = "uart0",
                .channels       = MAP(S3C2443_DMAREQSEL_UART0_1),
        },
        [DMACH_UART1_SRC2] = {
                .name           = "uart1",
                .channels       = MAP(S3C2443_DMAREQSEL_UART1_1),
        },
        [DMACH_UART2_SRC2] = {
                .name           = "uart2",
                .channels       = MAP(S3C2443_DMAREQSEL_UART2_1),
        },
        [DMACH_UART3_SRC2] = {
                .name           = "uart3",
                .channels       = MAP(S3C2443_DMAREQSEL_UART3_1),
        },
        [DMACH_TIMER] = {
                .name           = "timer",
                .channels       = MAP(S3C2443_DMAREQSEL_TIMER),
        },
        [DMACH_I2S_IN] = {
                .name           = "i2s-sdi",
                .channels       = MAP(S3C2443_DMAREQSEL_I2SRX),
        },
        [DMACH_I2S_OUT] = {
                .name           = "i2s-sdo",
                .channels       = MAP(S3C2443_DMAREQSEL_I2STX),
        },
        [DMACH_PCM_IN] = {
                .name           = "pcm-in",
                .channels       = MAP(S3C2443_DMAREQSEL_PCMIN),
        },
        [DMACH_PCM_OUT] = {
                .name           = "pcm-out",
                .channels       = MAP(S3C2443_DMAREQSEL_PCMOUT),
        },
        [DMACH_MIC_IN] = {
                .name           = "mic-in",
                .channels       = MAP(S3C2443_DMAREQSEL_MICIN),
        },
};

static void s3c2443_dma_select(struct s3c2410_dma_chan *chan,
			       struct s3c24xx_dma_map *map)
{
        writel(map->channels[0] | S3C2443_DMAREQSEL_HW,
               chan->regs + S3C2443_DMA_DMAREQSEL);
}

static struct s3c24xx_dma_selection __initdata s3c2443_dma_sel = {
	.select		= s3c2443_dma_select,
	.dcon_mask	= 0,
	.map		= s3c2443_dma_mappings,
	.map_size	= ARRAY_SIZE(s3c2443_dma_mappings),
};

static int __init s3c2443_dma_add(struct sys_device *sysdev)
{
	s3c24xx_dma_init(6, IRQ_S3C2443_DMA0, 0x100);
	return s3c24xx_dma_init_map(&s3c2443_dma_sel);
}

#if defined(CONFIG_CPU_S3C2416)
static struct sysdev_driver s3c2443_dma_driver __initdata = {
	.add	= s3c2443_dma_add,
};

static int __init s3c2443_dma_drvinit(void)
{
	return sysdev_driver_register(&s3c2416_sysclass, &s3c2443_dma_driver);
}

arch_initcall(s3c2443_dma_drvinit);
#endif

#if defined(CONFIG_CPU_S3C2442)
/* S3C2442 DMA contains the same selection table as the S3C2410 */
static struct sysdev_driver s3c2442_dma_driver = {
	.add	= s3c2410_dma_add,
};

static int __init s3c2442_dma_drvinit(void)
{
	return sysdev_driver_register(&s3c2442_sysclass, &s3c2442_dma_driver);
}

arch_initcall(s3c2442_dma_drvinit);
#endif

