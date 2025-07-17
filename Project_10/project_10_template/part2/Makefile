INCLUDES=-I../include
FLAGS=-Wall

part2: edgedetect.c
	gcc $(FLAGS) $(INCLUDES) -o part2 edgedetect.c ../physical.c

.PHONY:	clean

clean:
	rm -f *.bmp part2 ../physical.o
