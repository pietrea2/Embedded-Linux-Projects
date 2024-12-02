obj-m += video.o

all: video

video:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

.PHONY:	clean all

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

