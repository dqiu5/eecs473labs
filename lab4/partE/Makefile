obj-m := partE.o #nothing.o hello.o partC.o testPartC.o
MY_CFLAGS += -g -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
debug:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules EXTRA_CFLAGS="$(MY_CFLAGS)"
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
