obj-m += video.o

all: video part1

video:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

part1: part1.c
	gcc -g -Wall -o part1 part1.c

.PHONY:	all clean

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o part1

