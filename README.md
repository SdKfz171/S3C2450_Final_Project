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

#### 플랫폼 디바이스 드라이버 수정


### 각종 설정 파일 수정


## 리눅스 C 애플리케이션 작성

### 소켓 서버 프로그램

### 보드 소켓 클라이언트 프로그램
