INCLUDES=-I../include -I/usr/include/intelfpgaup/
FLAGS=-Wall

part4:	part4.c
	gcc $(FLAGS) $(INCLUDES) -o part4 part4.c ../physical.c -lintelfpgaup -lm -pthread

.PHONY:	clean

clean:
	rm -f part4

