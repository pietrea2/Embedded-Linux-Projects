obj-m += video.o

all: video part5

video:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

part5: part5.c
	gcc -g -Wall -o part5 part5.c

.PHONY:	clean all

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o part5

