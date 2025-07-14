part1: part1.o physical.o ../ADXL345.o
	gcc -I. -I../include  -Wall part1.o physical.o ../ADXL345.o -o part1

%.o: %.c
	gcc -I. -I../include -Wall -c $< -o $@

.PHONY:	clean

clean:
	rm -f *.o part1
	rm -f ../ADXL345.o
