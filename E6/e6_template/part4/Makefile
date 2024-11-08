obj-m += video.o

all: video part4

video:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

part4: part4.c
	gcc -g -Wall -o part4 part4.c

.PHONY:	clean all

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o part4

