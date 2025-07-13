obj-m += video.o

all: video part2

video:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

part2: part2.c
	gcc -g -Wall -o part2 part2.c

.PHONY:	clean all

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o part2

