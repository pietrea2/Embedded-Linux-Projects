INCLUDES=-I../include/ -I/usr/include/intelfpgaup/
FLAGS=-Wall

part5:	part5.c
	gcc $(FLAGS) $(INCLUDES) -o part5 ../stopwatch_wrappers.c part5.c ../physical.c -pthread -lintelfpgaup -lm


.PHONY:	clean

clean:
	rm -f part5

