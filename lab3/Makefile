# If called directly from the command line, invoke the kernel build system.
ifeq ($(KERNELRELEASE),)
 
    KERNEL_SOURCE := /usr/src/linux-headers-$(shell uname -r)
    PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) modules
 
clean:
	$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) clean
 
# Otherwise KERNELRELEASE is defined; we've been invoked from the
# kernel build system and can use its language.
else
 
    obj-m := pp_adc.o chardev.o
 
endif
