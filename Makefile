export ARCH=arm
#export CROSS_COMPILE=/home/jiangxiujie/a3333/lichee/buildroot/output/external-toolchain/bin/arm-linux-gnueabi-
export CROSS_COMPILE=/home/jiangxiujie/a3333/lichee/out/sun8iw5p1/android/common/buildroot/external-toolchain/bin/arm-linux-gnueabi-
EXTRA_CFLAGS += $(DEBFLAGS) -Wall

obj-m += key_input.o
#obj-m += adc_key.o
#obj-m += test/irq.o

KDIR ?= ~/a3333/lichee/linux-3.4
PWD := $(shell pwd)
all:
	make $(EXTRA_CFLAGS) -C $(KDIR) M=$(PWD) modules
clean:
	#git clean
	rm -rf *.o *.ko *.mod.c *.symvers modules.order .key_input* .tmp_versions
	#如何删除test目录下的垃圾文件
