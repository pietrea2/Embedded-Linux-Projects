obj-m += LEDR_HEX.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:  
    # find . -type f | xargs touch
    # rm -rf $(OBJS)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean