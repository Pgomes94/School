obj-m += keyboard_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) -Wl,-wrap,getchar -o keyboard_module_test keyboard_module_test.c
	insmod keyboard_module.ko

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm keyboard_module_test
	rmmod keyboard_module

