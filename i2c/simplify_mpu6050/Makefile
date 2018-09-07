ifneq ($(KERNELRELEASE),)
MODULE_NAME = mpu6050_module
$(MODULE_NAME)-objs := mpu6050.o
obj-m := $(MODULE_NAME).o
else
KERNEL_DIR = /lib/modules/`uname -r`/build
MODULEDIR := $(shell pwd)

.PHONY: modules
default: modules

modules:
	make -C $(KERNEL_DIR) M=$(MODULEDIR) modules

clean distclean:
	rm -f *.o *.mod.c .*.*.cmd *.ko
	rm -rf .tmp_versions
    
endif
