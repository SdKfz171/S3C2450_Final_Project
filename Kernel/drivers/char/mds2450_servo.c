/*
*	G2450_ADC.C - The s3c2450 adc module.
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <asm/delay.h>
#include <linux/io.h>
#include <plat/adc.h>
#include <plat/devs.h>
#include <linux/platform_device.h>
#include <mach/regs-gpio.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <plat/gpio-cfg.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define MDS2450_SERVO_MAJOR 74
static char servo_name[] = "mds2450-servo";

static DECLARE_WAIT_QUEUE_HEAD(servo_wq);

static int key_value = 0;

static void servo_R90(){
	gpio_set_value(S3C2410_GPG(15), 1);
	mdelay(2);		// 2ms delay
	gpio_set_value(S3C2410_GPG(15), 0);
	mdelay(18);	// 18ms delay
}

static void servo_R0(){
	gpio_set_value(S3C2410_GPG(15), 1);
	mdelay(1);
	udelay(500);	// 1.5ms delay
	gpio_set_value(S3C2410_GPG(15), 0);
	mdelay(18);
	udelay(500);	// 18.5ms delay
}

static ssize_t mds2450_servo_write(struct file * filp, const char * buf, size_t count, loff_t * pos){
    char * data;
    data = kmalloc(count, GFP_KERNEL);

    copy_from_user(data, buf, count);
    printk("%s\n", data);
    
    if(data[0] - '0')
    	servo_R90();
    else
    	servo_R0();
    kfree(data);
    return count;
}

static ssize_t mds2450_servo_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	int  ret;

	copy_to_user((void *)buff, (const void *)&key_value , sizeof( int ));
	ret = key_value;
	key_value = 0;
	
	return ret;
}

static int mds2450_servo_open(struct inode * inode, struct file * file)
{
	int ret = 0;
	int i;

	printk(KERN_INFO "ready to operation for servo\n");

	s3c_gpio_cfgpin(S3C2410_GPG(15), S3C_GPIO_SFN(1));	// EINT15,	LED4

	return ret;
}

static void mds2450_servo_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "end of servo\n");
}

static struct file_operations mds2450_servo_fops = {
	.owner 	= THIS_MODULE,
	.open 	= mds2450_servo_open,
	.release= mds2450_servo_release,
	.write 	= mds2450_servo_write,
	.read 	= mds2450_servo_read,
};

static int __devinit mds2450_servo_probe(struct platform_device *pdev)
{
	int ret;

	ret = register_chrdev( MDS2450_SERVO_MAJOR, servo_name, &mds2450_servo_fops );

    return ret;
}

static int __devexit mds2450_servo_remove(struct platform_device *pdev)
{
	unregister_chrdev( MDS2450_SERVO_MAJOR, servo_name );

	return 0;
}

static struct platform_driver mds2450_servo_device_driver = {
	.probe      = mds2450_servo_probe,
	.remove     = __devexit_p(mds2450_servo_remove),
	.driver     = {
		.name   = "mds2450-servo",
		.owner  = THIS_MODULE,
	}
};

static int __init mds2450_servo_init(void)
{
 	return platform_driver_register(&mds2450_servo_device_driver);
}

static void __exit mds2450_servo_exit(void)
{
	platform_driver_unregister(&mds2450_servo_device_driver);
}

module_init(mds2450_servo_init);
module_exit(mds2450_servo_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("servo for MDS2450");
