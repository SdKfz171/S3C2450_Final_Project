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

static void multitab_control_timer_handler(unsigned long data); 

#define MDS2450_MULTITAB_CONTROL_MAJOR 71
static char multitab_control_name[] = "mds2450-multitab_control";

static DECLARE_WAIT_QUEUE_HEAD(multitab_control_wq);

#define MULTITAB_CONTROL_TIME	(1*HZ)
static struct timer_list multitab_control_timer = TIMER_INITIALIZER(multitab_control_timer_handler, 0, 0);

static int key_value = 0;

static char * multitab_array;							// RELAY의 상태를 저장 할 배열
static char * multitab_array_old;						// RELAY의 이전 상태를 저장 할 배열
static int multitab_count = 2;							// RELAY 갯수

// 타이머 핸들러 함수
static void multitab_control_timer_handler(unsigned long data)
{
	int lp;
	int i;

	// RELAY 갯수만큼 반복하며 값 확인
	for(i = 0; i < multitab_count; i++){
		gpio_set_value(S3C2410_GPG(1 + i), multitab_array[i]);		// RELAY 
		gpio_set_value(S3C2410_GPG(4 + i), !multitab_array[i]);		// BOARD LED

		// RELAY의 상태가 이전 값이랑 다르면서
		if(multitab_array_old[i] != multitab_array[i]){
			if(multitab_array[i]) printk("Multitab %d ON\n", i);	// 1이면
			else printk("Multitab %d OFF\n", i);			// 0이면
		}

		multitab_array_old[i] = multitab_array[i];
	}

	mod_timer(&multitab_control_timer, jiffies + (MULTITAB_CONTROL_TIME));
}

// 파일 오퍼레이션 write 함수
static ssize_t mds2450_multitab_control_write(struct file * filp, const char * buf, size_t count, loff_t * pos){
    char * data;								// 유저에게서 받은 값을 저장 할 버퍼
    data = kmalloc(count, GFP_KERNEL);						// 버퍼에 동적 할당 

    copy_from_user(data, buf, count);						// 유저로 부터 값 복사
    printk("%s\n", data);
    
	multitab_array[data[0] - '0'] = data[1] - '0';				// RELAY 상태 배열에 유저로 받은 값 반영

    kfree(data);								// 버퍼 메모리 해제
    return count;
}

// 파일 오퍼레이션 read 함수
static ssize_t mds2450_multitab_control_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	int  ret;
	
	// user로 값을 값을 전송
	copy_to_user((void *)buff, (const void *)&key_value , sizeof(int));
	ret = key_value;
	key_value = 0;
	
	return ret;
}

// 파일 오퍼레이션 open 함수
static int mds2450_multitab_control_open(struct inode * inode, struct file * file)
{
	int ret = 0;
	int i;

	printk(KERN_INFO "ready to scan key value\n");

	// RELAY 상태 배열 동적 할당
	multitab_array = kmalloc(multitab_count, GFP_KERNEL);
	multitab_array_old = kmalloc(multitab_count, GFP_KERNEL);

	// 실행과 함께 상태 초기화
	for(i = 0; i < multitab_count; i++){
		multitab_array[i] = 0;	
		multitab_array_old[i] = multitab_array[i];
	}

	// GPIO Initial
	s3c_gpio_cfgpin(S3C2410_GPG(1), S3C_GPIO_SFN(1));			// EINT9,	RELAY1
	s3c_gpio_cfgpin(S3C2410_GPG(2), S3C_GPIO_SFN(1));			// EINT10,	RELAY2
	s3c_gpio_cfgpin(S3C2410_GPG(3), S3C_GPIO_SFN(1));			// EINT11,	RELAY3
	s3c_gpio_cfgpin(S3C2410_GPG(4), S3C_GPIO_SFN(1));			// EINT12,	LED1
	s3c_gpio_cfgpin(S3C2410_GPG(5), S3C_GPIO_SFN(1));			// EINT13,	LED2
	s3c_gpio_cfgpin(S3C2410_GPG(6), S3C_GPIO_SFN(1));			// EINT14,	LED3
	s3c_gpio_cfgpin(S3C2410_GPG(7), S3C_GPIO_SFN(1));			// EINT15,	LED4

	// Scan timer
	mod_timer(&multitab_control_timer, jiffies + (MULTITAB_CONTROL_TIME));

	return ret;
}

// 파일 오퍼레이션 release 함수
static void mds2450_multitab_control_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "end of the scanning\n");

	// 동적 할당한 배열 메모리 해제 
	kfree(multitab_array);								
	kfree(multitab_array_old);							

	del_timer_sync(&multitab_control_timer);				// 타이머 핸들러 해제
}

// 현재 디바이스 드라이버의 파일 오퍼레이션 구조체
static struct file_operations mds2450_multitab_control_fops = {
	.owner 	= THIS_MODULE,
	.open 	= mds2450_multitab_control_open,
	.release= mds2450_multitab_control_release,
	.write 	= mds2450_multitab_control_write,
	.read 	= mds2450_multitab_control_read,
};

// 캐릭터 디바이스 드라이버 관련 초기화 함수
static int __devinit mds2450_multitab_control_probe(struct platform_device *pdev)
{
	int ret;

	ret = register_chrdev( MDS2450_MULTITAB_CONTROL_MAJOR, multitab_control_name, &mds2450_multitab_control_fops );

    return ret;
}

static int __devexit mds2450_multitab_control_remove(struct platform_device *pdev)
{
	unregister_chrdev( MDS2450_MULTITAB_CONTROL_MAJOR, multitab_control_name );

	return 0;
}

// 플랫폼 디바이스 드라이버 관련 구조체
static struct platform_driver mds2450_multitab_control_device_driver = {
	.probe      = mds2450_multitab_control_probe,
	.remove     = __devexit_p(mds2450_multitab_control_remove),
	.driver     = {
		.name   = "mds2450-multitab_control",
		.owner  = THIS_MODULE,
	}
};

// 모듈 초기화 코드 
static int __init mds2450_multitab_control_init(void)
{
 	return platform_driver_register(&mds2450_multitab_control_device_driver);
}

// 모듈 해제 코드
static void __exit mds2450_multitab_control_exit(void)
{
	platform_driver_unregister(&mds2450_multitab_control_device_driver);
}

// 모듈의 시작
module_init(mds2450_multitab_control_init);
module_exit(mds2450_multitab_control_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("led multitab_control for MDS2450");
