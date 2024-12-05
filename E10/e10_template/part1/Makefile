INCLUDES=-I../include -I/usr/include/intelfpgaup
FLAGS=-g -Wall

.PHONY:	clean

part1: edgedetect.c
	gcc $(FLAGS) $(INCLUDES) -o part1 edgedetect.c -lintelfpgaup -lm

clean:
	rm -f part1 *.bmp
