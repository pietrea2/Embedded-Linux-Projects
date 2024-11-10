obj-m += video.o

all: video part3

video:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

part3: part3.c
	gcc -g -Wall -o part3 part3.c

.PHONY:	clean all

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o part3

