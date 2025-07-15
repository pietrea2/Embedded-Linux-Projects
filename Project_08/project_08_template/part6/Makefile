INCLUDES=-I../include -I/usr/include/intelfpgaup/
FLAGS=-Wall

part6:	part6.c
	gcc $(FLAGS) $(INCLUDES) -o part6 ../stopwatch_wrappers.c part6.c -pthread -lintelfpgaup -lm

.PHONY:	clean

clean:
	rm -f part6

