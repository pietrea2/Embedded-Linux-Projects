INCLUDES=-I../include
FLAGS= -Wall

part1: physical.c part1.c
	gcc $(FLAGS) $(INCLUDES) part1.c physical.c -o part1

clean:
	find . -type f | xargs touch
	rm -f part1 *.o

