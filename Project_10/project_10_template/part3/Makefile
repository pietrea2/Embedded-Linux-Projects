INCLUDES=-I../include
FLAGS=-Wall

part3: edgedetect.c
	gcc $(FLAGS) $(INCLUDES) -o part3 edgedetect.c ../physical.c

.PHONY:	clean

clean:
	rm -f *.bmp part3 ../physical.o
