/*
 *
 * linux/arch/arm/mach-s3c2416/mach-mds2450.c
 *
 * Copyright (c) 2009 Yauhen Kharuzhy <jekhor@gmail.com>,
 *  as part of OpenInkpot project
 * Copyright (c) 2009 Promwad Innovation Company
 *  Yauhen Kharuzhy <yauhen.kharuzhy@promwad.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/smsc911x.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <linux/input.h>
#include <linux/fb.h>
#include <linux/delay.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <video/platform_lcd.h>

#include <linux/usb/gpio_vbus.h>


#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>
#include <mach/regs-lcd.h>
#include <mach/regs-s3c2443-clock.h>
#include <mach/regs-s3c2412-mem.h>

#include <mach/idle.h>
#include <mach/leds-gpio.h>
#include <plat/iic.h>

#include <plat/adc.h>
#include <plat/s3c2416.h>
#include <plat/gpio-cfg.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/nand.h>
#include <plat/sdhci.h>
#include <plat/udc.h>
#include <plat/ts.h>

#include <plat/regs-fb-v4.h>
#include <plat/fb.h>
#include <plat/pm.h>




static struct map_desc mds2450_iodesc[] __initdata = {
    /* ISA IO Space map (memory space selected by A24) */
};

#define UCON (S3C2410_UCON_DEFAULT  | \
        S3C2440_UCON_PCLK   | \
        S3C2443_UCON_RXERR_IRQEN)

#define ULCON (S3C2410_LCON_CS8 | S3C2410_LCON_PNONE)

#define UFCON (S3C2410_UFCON_RXTRIG8    | \
        S3C2410_UFCON_FIFOMODE  | \
        S3C2440_UFCON_TXTRIG16)

static struct s3c2410_uartcfg mds2450_uartcfgs[] __initdata = {
    [0] = {
        .hwport      = 0,
        .flags       = 0,
        .ucon        = UCON,
        .ulcon       = ULCON,
        .ufcon       = UFCON,
    },
    [1] = {
        .hwport      = 1,
        .flags       = 0,
        .ucon        = UCON,
        .ulcon       = ULCON,
        .ufcon       = UFCON,
    },
    [2] = {
        .hwport      = 2,
        .flags       = 0,
        .ucon        = UCON,
        .ulcon       = ULCON,
        .ufcon       = UFCON,
    },
    [3] = {
        .hwport      = 3,
        .flags       = 0,
        .ucon        = UCON,
        .ulcon       = ULCON,
        .ufcon       = UFCON,
    }
};

void mds2450_hsudc_gpio_init(void)
{
    s3c_gpio_setpull(S3C2410_GPH(14), S3C_GPIO_PULL_UP);
    s3c_gpio_cfgpin(S3C2410_GPH(14), S3C_GPIO_SFN(1));
    s3c2410_modify_misccr(S3C2416_MISCCR_SEL_SUSPND, 0);
}

void mds2450_hsudc_gpio_uninit(void)
{
    s3c2410_modify_misccr(S3C2416_MISCCR_SEL_SUSPND, 1);
    s3c_gpio_setpull(S3C2410_GPH(14), S3C_GPIO_PULL_NONE);
    s3c_gpio_cfgpin(S3C2410_GPH(14), S3C_GPIO_SFN(0));
}

struct s3c24xx_hsudc_platdata mds2450_hsudc_platdata = {
    .epnum = 9,
    .gpio_init = mds2450_hsudc_gpio_init,
    .gpio_uninit = mds2450_hsudc_gpio_uninit,
};

static struct gpio_vbus_mach_info mds24_hsudc_vbus_pdata = {
        .gpio_vbus              = S3C2410_GPG(13),
        .gpio_pullup            = -1,
        .gpio_vbus_inverted     = true,
};

static struct platform_device mds24_hsudc_vbus_dev = {
        .name                   = "gpio-vbus",
        .dev.platform_data      = &mds24_hsudc_vbus_pdata,
};


/* NAND parititon */
static struct mtd_partition mds2450_default_nand_part[] = {
        [0] = {
                .name           = "Bootloader",
                .offset         = 0,
                .size           = (512*SZ_1K),
        //        .mask_flags     = MTD_CAP_NANDFLASH,
        },
        [1] = {
                .name           = "Kernel",
                .offset         = (512*SZ_1K),    /* Block number is 0x10 */
                .size           = (5*SZ_1M) - (512*SZ_1K),
        //        .mask_flags     = MTD_CAP_NANDFLASH,
        },
        [2] = {
                .name           = "rootfs",
                .offset         = MTDPART_OFS_APPEND,
                .size           = MTDPART_SIZ_FULL,
        },
};

static struct s3c2410_nand_set mds2450_nand_sets[] = {
    [0] = {
            .name           = "NAND",
            .nr_chips       = 1,
            .nr_partitions  = ARRAY_SIZE(mds2450_default_nand_part),
            .partitions     = mds2450_default_nand_part,
    },
};

/* choose a set of timings which should suit most 512Mbit
 * chips and beyond.
*/

static struct s3c2410_platform_nand mds2450_nand_info = {
    .tacls          = 20,
    .twrph0         = 40,
    .twrph1         = 20,
    .nr_sets        = ARRAY_SIZE(mds2450_nand_sets),
    .sets           = mds2450_nand_sets,
};

static struct resource mds2450_smsc911x_resources[] = {
    [0] = {
            .start = S3C2410_CS5,
            .end   = S3C2410_CS5 + SZ_64K - 1,
            .flags = IORESOURCE_MEM,
    },
    [1] = {
            .start = IRQ_EINT18,
            .end   = IRQ_EINT18,
            .flags = IORESOURCE_IRQ | IRQ_TYPE_LEVEL_LOW,
    },
};

static struct smsc911x_platform_config mds2450_smsc911x_pdata = {
    .irq_polarity  = SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
    .irq_type      = SMSC911X_IRQ_TYPE_PUSH_PULL,
    .flags         = SMSC911X_USE_16BIT | SMSC911X_FORCE_INTERNAL_PHY,
    .phy_interface = PHY_INTERFACE_MODE_MII,
    .mac           = {0x00, 0x80, 0x00, 0x23, 0x45, 0x67},
};


static struct platform_device mds2450_smsc911x = {
    .name          = "smsc911x",
    .id            = -1,
    .num_resources = ARRAY_SIZE(mds2450_smsc911x_resources),
    .resource      = &mds2450_smsc911x_resources[0],
    .dev = {
            .platform_data = &mds2450_smsc911x_pdata,
    },
};
/* kimsy */
static struct platform_device mds2450key_device = {
    .name = "mds2450_key",
        .id   = -1,
};

static int __init ethaddr_setup(char *line)
{
    char *ep;
    int i;

    /* there should really be routines to do this stuff */
    for (i = 0; i < 6; i++) {
        mds2450_smsc911x_pdata.dev_addr[i] = line ? simple_strtoul(line, &ep, 16) : 0;
        if (line)
            line = (*ep) ? ep+1 : ep;
    }
    printk("User MAC address: %pM\n", mds2450_smsc911x_pdata.dev_addr);
    return 0;
}
__setup("ethaddr=", ethaddr_setup);

struct s3c_fb_pd_win mds2450_fb_win[] = {
    [0] = { /* Innolux 4.3inch */
        .win_mode       = {
                .pixclock       = 9000,
                .upper_margin   = 10,
                .lower_margin   = 1,
                .vsync_len      = 1,
                .left_margin    = 43,
                .right_margin   = 1,
                .hsync_len      = 1,
                .xres           = 480,
                .yres           = 272,
        },
        .default_bpp    = 16,
        .max_bpp        = 32,
    },
};

static void s3c2416_fb_gpio_setup_24bpp(void)
{
    unsigned int gpio;

    for (gpio = S3C2410_GPC(1); gpio <= S3C2410_GPC(4); gpio++) {
        s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
        s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
    }

    for (gpio = S3C2410_GPC(8); gpio <= S3C2410_GPC(15); gpio++) {
        s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
        s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
    }

    for (gpio = S3C2410_GPD(0); gpio <= S3C2410_GPD(15); gpio++) {
        s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
        s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
    }
}

static struct s3c_fb_platdata mds2450_fb_platdata = {
    .win[0]      = &mds2450_fb_win[0],
    .setup_gpio  = s3c2416_fb_gpio_setup_24bpp,
    .vidcon0     = VIDCON0_VIDOUT_RGB_2443 | VIDCON0_PNRMODE_BGR_2443,
    .vidcon1     = VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
};

static struct s3c_sdhci_platdata mds2450_hsmmc0_pdata __initdata = {
    .max_width      = 4,

    .cd_type        = S3C_SDHCI_CD_GPIO,
    .ext_cd_gpio        = S3C2410_GPG(8),       // Gemini 2013.12.26
    .ext_cd_gpio_invert = 1,
};

static struct s3c_sdhci_platdata mds2450_hsmmc1_pdata __initdata = {
    .max_width      = 4,
    .cd_type        = S3C_SDHCI_CD_INTERNAL,
};

static struct platform_device mds2450_device_kscan = {
    .name = "mds2450-kscan",
};

static struct platform_device mds2450_device_blinky = {
    .name = "mds2450-blinky"
};


static struct platform_device mds2450_device_multitab_control = {
    .name = "mds2450-multitab_control"
};

static struct platform_device mds2450_device_servo = {
    .name = "mds2450-servo"
};

static struct i2c_board_info mds2450_i2c0_board_info[] __initdata = {
    {
        I2C_BOARD_INFO("wm8960", 0x1a),
    }
};

static void mds2450_innolux43_set_power(struct plat_lcd_data *pd,
                                        unsigned int power)
{
    if (power) {
#if !defined(CONFIG_BACKLIGHT_PWM)
        gpio_request(S3C2410_GPB(0), "Display Backlight");
        gpio_direction_output(S3C2410_GPB(0), 1);
        gpio_free(S3C2410_GPB(0));
#endif

        /* LCD POWER Enable */
        gpio_request(S3C2410_GPG(12), "Display POWER");
        gpio_direction_output(S3C2410_GPG(12), 1);
        gpio_free(S3C2410_GPG(12));
    } else {
#if !defined(CONFIG_BACKLIGHT_PWM)
        gpio_request(S3C2410_GPB(0), "Display Backlight");
        gpio_direction_output(S3C2410_GPB(0), 0);
        gpio_free(S3C2410_GPB(0));
#endif
    }
}

static struct plat_lcd_data mds2450_lcd_innolux43_data = {
    .set_power      = mds2450_innolux43_set_power,
};

static struct platform_device mds2450_lcd_innolux43 = {
    .name                   = "platform-lcd",
    .dev.parent             = &s3c_device_fb.dev,
    .dev.platform_data      = &mds2450_lcd_innolux43_data,
};

static struct s3c2410_ts_mach_info s3c_ts_platform __initdata = {
    .delay                  = 10000,
    .presc                  = 49,
    .oversampling_shift     = 2,
};

static struct platform_device *mds2450_devices[] __initdata = {
    &s3c_device_adc,
    &s3c_device_fb,
    &s3c_device_rtc,
    &s3c_device_wdt,
    &s3c_device_ohci,
    &s3c_device_nand,
    &s3c_device_i2c0,
    &s3c_device_ts,
    &s3c_device_hsmmc0,
    &s3c_device_hsmmc1,
    &s3c_device_usb_hsudc,
    &s3c_device_timer[0], // Gemini 2014.01.15
    &s3c_device_timer[1], // Gemini 2014.01.15
    &mds24_hsudc_vbus_dev,
    &s3c2416_device_iis,
    &samsung_asoc_dma,
    &mds2450_smsc911x,
//    &mds2450key_device, /* kimsy */
    &mds2450_device_kscan,
    &mds2450_device_blinky,
    &mds2450_device_multitab_control,
    &mds2450_device_servo,
    &mds2450_lcd_innolux43,
};

static void __init mds2450_map_io(void)
{
    s3c24xx_init_io(mds2450_iodesc, ARRAY_SIZE(mds2450_iodesc));
    s3c24xx_init_clocks(12000000);
    s3c24xx_init_uarts(mds2450_uartcfgs, ARRAY_SIZE(mds2450_uartcfgs));
}

static void mds2450_board_gpio_setup(void)
{
        unsigned int gpio;

/* GPB1 PWM */
        s3c_gpio_cfgpin(S3C2410_GPG(1), S3C_GPIO_SFN(2));
        s3c_gpio_setpull(S3C2410_GPG(1), S3C_GPIO_PULL_UP);
/* GPB3 TOUT3 LCD_BRIGHTNESS_CTL */
        for (gpio = S3C2410_GPB(3); gpio <= S3C2410_GPB(4); gpio++) {
                s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(1));
                s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
        }

/// Gemini 2013.12.26
/* GPG4 EINT12 LCD_PWEREN */
        s3c_gpio_cfgpin(S3C2410_GPG(4), S3C_GPIO_SFN(1));
        s3c_gpio_setpull(S3C2410_GPG(4), S3C_GPIO_PULL_NONE);
    
/// Gemini 2014.01.22
/* GPG6 EINT11 HP_DETECT */
        s3c_gpio_cfgpin(S3C2410_GPG(11), S3C_GPIO_SFN(2));
        s3c_gpio_setpull(S3C2410_GPG(11), S3C_GPIO_PULL_NONE);

/// Gemini 2013.12.26
/* GPG8 EINT17 PWRENABLE_USBH */
        s3c_gpio_cfgpin(S3C2410_GPG(9), S3C_GPIO_SFN(1));
        s3c_gpio_setpull(S3C2410_GPG(9), S3C_GPIO_PULL_NONE);

/* GPG9 EINT17 BT_EN */
        s3c_gpio_cfgpin(S3C2410_GPG(9), S3C_GPIO_SFN(1));
        s3c_gpio_setpull(S3C2410_GPG(9), S3C_GPIO_PULL_NONE);

/* GPG10 EINT18 IRQ_LAN */
        s3c_gpio_cfgpin(S3C2410_GPG(10), S3C_GPIO_SFN(2));
        s3c_gpio_setpull(S3C2410_GPG(10), S3C_GPIO_PULL_UP);
        
/* GPG13 EINT21 VBUS_DETECT */
        s3c_gpio_cfgpin(S3C2410_GPG(13), S3C_GPIO_SFN(2));
        s3c_gpio_setpull(S3C2410_GPG(13), S3C_GPIO_PULL_NONE);
}

static void __init mds2450_smsc9220_init(void)
{
    u32 cs5;

    /* configure nCS5 width to 16 bits */
    __raw_writel(0xF, S3C2412_SMIDCYR(5));
    __raw_writel(0x1F, S3C2412_SMBWSTRD(5));
    __raw_writel(0x1F, S3C2412_SMBWSTWRR(5));
    __raw_writel(4, S3C2412_SMBWSTOENR(5));
    __raw_writel(4, S3C2412_SMBWSTWENR(5));

    cs5 = __raw_readl(S3C2412_SMBCR(5));
    cs5 |= (1<<2)|(1<<0);
    cs5 &= ~((3<<20)|(3<<12));
    cs5 &= ~(3<<4);
    cs5 |= (1<<4);
    __raw_writel(cs5, S3C2412_SMBCR(5));
/*
    __raw_writel(0x0, S3C2412_SMIDCYR(5));
    __raw_writel(14, S3C2412_SMBWSTRD(5));
    __raw_writel(14, S3C2412_SMBWSTWRR(5));
    __raw_writel(4, S3C2412_SMBWSTOENR(5));
    __raw_writel(4, S3C2412_SMBWSTWENR(5));

    cs5 = __raw_readl(S3C2412_SMBCR(5));
    cs5 |= (1<<2)|(1<<0);
    cs5 &= ~((3<<20)|(3<<12));
    cs5 &= ~(3<<4);
    cs5 |= (1<<4);
    __raw_writel(cs5, S3C2412_SMBCR(5));
*/
}

static void __init mds2450_machine_init(void)
{
    mds2450_board_gpio_setup();
    mds2450_smsc9220_init();

    s3c_i2c0_set_platdata(NULL);
    i2c_register_board_info(0, mds2450_i2c0_board_info,
            ARRAY_SIZE(mds2450_i2c0_board_info));
    s3c_fb_set_platdata(&mds2450_fb_platdata);

    s3c24xx_ts_set_platdata(&s3c_ts_platform);


    s3c_sdhci0_set_platdata(&mds2450_hsmmc0_pdata); // Delete
    s3c_sdhci1_set_platdata(&mds2450_hsmmc1_pdata);
    s3c_nand_set_platdata(&mds2450_nand_info);

    s3c24xx_hsudc_set_platdata(&mds2450_hsudc_platdata);

    gpio_request(S3C2410_GPG(9), "USBH_PWREN");
    gpio_direction_output(S3C2410_GPG(9), 1);
    gpio_free(S3C2410_GPG(9));


    platform_add_devices(mds2450_devices, ARRAY_SIZE(mds2450_devices));

    s3c_pm_init();
}

MACHINE_START(MDS2450, "MDS2450")
    .boot_params    = S3C2410_SDRAM_PA + 0x100,
    .init_irq       = s3c24xx_init_irq,
    .map_io         = mds2450_map_io,
    .init_machine   = mds2450_machine_init,
    .timer          = &s3c24xx_timer,
MACHINE_END
