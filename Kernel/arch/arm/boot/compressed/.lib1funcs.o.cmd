cmd_arch/arm/boot/compressed/lib1funcs.o := arm-none-linux-gnueabi-gcc -Wp,-MD,arch/arm/boot/compressed/.lib1funcs.o.d  -nostdinc -isystem /root/arm-2010q1/bin/../lib/gcc/arm-none-linux-gnueabi/4.4.1/include -I/root/kernel-mds2450-3.0.22/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include include/generated/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-s3c2410/include -Iarch/arm/mach-s3c2400/include -Iarch/arm/mach-s3c2412/include -Iarch/arm/mach-s3c2416/include -Iarch/arm/mach-s3c2440/include -Iarch/arm/mach-s3c2443/include -Iarch/arm/plat-s3c24xx/include -Iarch/arm/plat-samsung/include -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables  -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm9tdmi -include asm/unified.h -msoft-float -gdwarf-2    -Wa,-march=all     -c -o arch/arm/boot/compressed/lib1funcs.o arch/arm/boot/compressed/lib1funcs.S

source_arch/arm/boot/compressed/lib1funcs.o := arch/arm/boot/compressed/lib1funcs.S

deps_arch/arm/boot/compressed/lib1funcs.o := \
    $(wildcard include/config/aeabi.h) \
  /root/kernel-mds2450-3.0.22/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  /root/kernel-mds2450-3.0.22/arch/arm/include/asm/linkage.h \
  /root/kernel-mds2450-3.0.22/arch/arm/include/asm/assembler.h \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/smp.h) \
  /root/kernel-mds2450-3.0.22/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /root/kernel-mds2450-3.0.22/arch/arm/include/asm/hwcap.h \
  /root/kernel-mds2450-3.0.22/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
    $(wildcard include/config/cpu/use/domains.h) \
  /root/kernel-mds2450-3.0.22/arch/arm/include/asm/unwind.h \
    $(wildcard include/config/arm/unwind.h) \

arch/arm/boot/compressed/lib1funcs.o: $(deps_arch/arm/boot/compressed/lib1funcs.o)

$(deps_arch/arm/boot/compressed/lib1funcs.o):
