obj-m += arbitrary_write.o

PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	male -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	