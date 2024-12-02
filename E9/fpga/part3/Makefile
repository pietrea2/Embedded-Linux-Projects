INCLUDES=-I../include -I/usr/include/intelfpgaup
FLAGS=-Wall

.PHONY: clean

part3: part3.c
	gcc $(FLAGS) $(INCLUDES) -o part3 part3.c ../physical.c -lrt -lintelfpgaup

clean:
	rm -f part3
