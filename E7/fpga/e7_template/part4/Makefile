obj-m += accel.o
accel-objs += accel_kernel_module.o ../ADXL345.o

all: accel part4

accel:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules KCPPFLAGS="-DKERNEL"

../ADXL345.o: ../ADXL345.c
	gcc -I. -I../include -Wall -DKERNEL -c ../ADXL345.c -o ../ADXL345.o

part4: part4.c ../accel_wrappers.c
	gcc -I. -I../include -Wall -o part4 ../accel_wrappers.c part4.c

.PHONY: clean accel all

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o part4
	rm -f ../ADXL345.o
