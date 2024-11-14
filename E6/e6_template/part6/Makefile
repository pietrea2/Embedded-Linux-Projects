obj-m += video.o

all: video part6

video:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

part6: part6.c
	gcc -g -Wall -o part6 part6.c -lintelfpgaup

.PHONY:	clean all

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o part6

