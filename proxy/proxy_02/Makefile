obj-m := driver.o
ROOTDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(ROOTDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(ROOTDIR) M=$(PWD) clean
