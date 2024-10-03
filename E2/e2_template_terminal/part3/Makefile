obj-m += stopwatch.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	find . -type f | xargs touch
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

