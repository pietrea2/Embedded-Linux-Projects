obj-m += chardev.o

.PHONY: all driver clean

all: driver part1

driver:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

part1: part1.c
	gcc -wall -o part1 part1.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o part1
