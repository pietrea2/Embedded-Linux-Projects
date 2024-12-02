INCLUDES=-I../include -I/usr/include/intelfpgaup
FLAGS=-Wall

.PHONY: clean

part4: part4.c
	gcc $(FLAGS) $(INCLUDES) -o part4 part4.c ../physical.c -lrt -lintelfpgaup

clean:
	rm -f part4
