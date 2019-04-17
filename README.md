# S3C2450_Final_Project

한컴MDS 임베디드 시스템 소프트웨어 양성과정 최종 프로젝트

## NFS 개발환경 수정

기존에는 PC에서 보드로 이더넷 케이블을 연결하여 직접 IP를 할당하는 방식으로 NFS를 사용하였다.

하지만, 그렇게 되면 네트워크를 사용 할 수 없기 떄문에 소켓 통신을 하지 못한다.

그래서 기존의 IP 할당 하는 방식에서 실제 네트워크에 연결하는 방식으로 변경을 하였다.

### VMware 네트워크 설정 변경

먼저 VMware의 우분투 가상머신의 네트워크 어댑터를 NAT에서 Bridged로 변경한다. (혹은 추가)

변경 후 우분투 가상머신의 네트워크 인터페이스 (Bridged로 연결한 네트워크 어댑터를 사용한다)

![](./Image/Untitled-c398e925-d0b8-4d9c-8fc8-484ee5764f47.png)

### U-Boot 설정 변경

ipaddr == 보드의 IP

serverip == PC의 IP

gatewayip == 보드와 PC에서 연결한 네트워크의 게이트웨이 IP

netmask == 넷 마스크

bootargs의 구성

    root=/dev/nfs rw nfsroot=<serverip>:<파일 시스템 디렉터리> ip=<ipaddr>:192.168.:<gatewayip>:<netmask>::eth0:on console=ttySAC1,115200n81

기존 U-Boot 설정 값

    set ipaddr 192.168.0.3
    set serverip 192.168.0.2
    set gatewayip 192.168.0.1
    setenv bootargs "root=/dev/nfs rw nfsroot=192.168.0.2:/tftpboot/rootfs/rootfs ip=192.168.0.3:192.168.:192.168.0.1:255.255.255.0::eth0:on console=ttySAC1,115200n81"
    saveenv

바꾼 U-Boot 설정 값

    set ipaddr 192.168.103.3
    set serverip 192.168.103.99
    set gatewayip 192.168.103.1
    setenv bootargs "root=/dev/nfs rw nfsroot=192.168.103.99:/tftpboot/rootfs/rootfs ip=192.168.103.3:192.168.:192.168.103.1:255.255.255.0::eth0:on console=ttySAC1,115200n81"
    saveenv

※ 당연하겠지만 보드와 PC를 같은 네트워크 상에 연결하는 것이 중요하다.

### 보드 재 부팅

    tftp 32000000 zImage; bootm 32000000

NFS 설정을 바꾸고 난 뒤에 기존에 파일시스템이 뻗는 일이 잦았는데 이제는 그런 경우가 없어졌다.


## 멀티탭 개조

![](./Image/릴레이.jpg)

멀티탭 선의 피복을 벗기고 3개의 선중에 갈색 상전압선의 중앙을 자르고 양쪽의 피복을 벗겨서 

한 쪽은 NO(노말 오픈), 한쪽은 COM(공통 단자)에 연결한다. 선을 연결 할 때 실수를 하게 되면 위험하므로 주의가 필요하다.


## 커널 코드 수정

커널 코드에서 수정해야 할 일은 디바이스 드라이버 수정이다.

그 다음 수정 한 값에 맞게 설정 값들을 수정 해주는 것이 필수이다.

### 디바이스 드라이버 작성

먼저 캐릭터 디바이스 드라이버를 새로 하나 만들고 플랫폼 디바이스 드라이버에서 해당 디바이스 드라이버를 추가해준다.

#### 캐릭터 디바이스 드라이버 작성

module_init(), module_exit()

모듈의 코드는 위의 부분에서 시작하고 끝난다.

그러므로 위의 로직을 먼저 구현 해주어야 한다. 인자는 함수 포인터이다.

```c
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
```

그 다음에 이 캐릭터 디바이스 드라이버를 플랫폼 디바이스 드라이버로 활용 할 것이기 떄문에 해당 구조체를 만들어준다.
```c
// 플랫폼 디바이스 드라이버 관련 구조체
static struct platform_driver mds2450_multitab_control_device_driver = {
	.probe      = mds2450_multitab_control_probe,
	.remove     = __devexit_p(mds2450_multitab_control_remove),
	.driver     = {
		.name   = "mds2450-multitab_control",
		.owner  = THIS_MODULE,
	}
};
```

플랫폼 디바이스 드라이버에서는 init과 exit과 같은 역할을 probe와 remove가 한다.

probe에서는 캐릭터 디바이스 드라이버를 등록하고 remove에서는 해제해주는 코드를 작성한다.

```c
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
```

여기서 중요한 것은 register_chrdev함수의 인자인 메이져 넘버와 파일 오퍼레이션 구조체이다.

모든 디바이스 드라이버 파일에는 메이저 넘버와 마이너 넘버가 존재한다.

```c
#define MDS2450_MULTITAB_CONTROL_MAJOR 71
```

메이저 넘버는 제어하려는 디바이스를 구분하기위한 ID이다. 당연하겠지만 각각의 디바이스 드라이버마다 메이저 넘버가 달라야 한다.

마이너 넘버는 해당 디바이스 내부에서 더 상세한 실제 디바이스를 구분 하기 위한 ID이다. 예를 들면 시리얼에서 USART0과 USART1을 구분 하는 것과 같다.  

```c
// 현재 디바이스 드라이버의 파일 오퍼레이션 구조체
static struct file_operations mds2450_multitab_control_fops = {
	.owner 	= THIS_MODULE,
	.open 	= mds2450_multitab_control_open,
	.release= mds2450_multitab_control_release,
	.write 	= mds2450_multitab_control_write,
	.read 	= mds2450_multitab_control_read,
};
```

다음으로 파일 오퍼레이션 구조체은 간단하게 우리가 파일 디스크립터를 열 때 사용하는 open, close, write, read 등의 함수들을 

내 입맛에 맞게 정의하여 사용할 수 있게 해주는 구조체이다. 

open과 같이 사용 할 때 사용자가 정의한 open함수에 연결하기 위해서 사용자 정의 함수의 함수 포인터를 이용한다.

```c
// 파일 오퍼레이션 write 함수
static ssize_t mds2450_multitab_control_write(struct file * filp, const char * buf, size_t count, loff_t * pos){
    char * data;										// 유저에게서 받은 값을 저장 할 버퍼
    data = kmalloc(count, GFP_KERNEL);					// 버퍼에 동적 할당 

    copy_from_user(data, buf, count);					// 유저로 부터 값 복사
    printk("%s\n", data);
    
	multitab_array[data[0] - '0'] = data[1] - '0';		// RELAY 상태 배열에 유저로 받은 값 반영

    kfree(data);										// 버퍼 메모리 해제
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
	s3c_gpio_cfgpin(S3C2410_GPG(1), S3C_GPIO_SFN(1));	// EINT9,	RELAY1
	s3c_gpio_cfgpin(S3C2410_GPG(2), S3C_GPIO_SFN(1));	// EINT10,	RELAY2
	s3c_gpio_cfgpin(S3C2410_GPG(3), S3C_GPIO_SFN(1));	// EINT11,	RELAY3
	s3c_gpio_cfgpin(S3C2410_GPG(4), S3C_GPIO_SFN(1));	// EINT12,	LED1
	s3c_gpio_cfgpin(S3C2410_GPG(5), S3C_GPIO_SFN(1));	// EINT13,	LED2
	s3c_gpio_cfgpin(S3C2410_GPG(6), S3C_GPIO_SFN(1));	// EINT14,	LED3
	s3c_gpio_cfgpin(S3C2410_GPG(7), S3C_GPIO_SFN(1));	// EINT15,	LED4

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

	del_timer_sync(&multitab_control_timer);			// 타이머 핸들러 해제
}
```
디바이스 드라이버 파일을 open 할 때에는 각종 초기화를 하고 release 할 때에는 할당받은 것들의 해제를 한다.

write를 할 때에는 user의 커널로 복사하고 read를 할 때에는 커널의 값을 user로 복사한다.

##### 상세기능
- open 함수에서는 GPIO 핀을 설정하고 타이머를 세팅 함과 동시에 현재 RELAY(멀티탭)의 상태를 저장 할 배열을 동적할당 했다.

- release 함수에서는 동적할당 한 배열과 타이머를 해제하였다. 동적 할당한 메모리를 해제하지 않으면 메모리 충돌이 일어 날 수 있다.

- write 함수에서는 user로 부터 받은 커맨드를 복사하여 값에 따라 RELAY(멀티탭) 상태 배열의 값을 바꿔준다.

- read 함수를 만들어두긴 했으나 사용을 하지는 않았다. 

#### 플랫폼 디바이스 드라이버 수정


### 각종 설정 파일 수정


## 리눅스 C 애플리케이션 작성

### 소켓 서버 프로그램

### 보드 소켓 클라이언트 프로그램
