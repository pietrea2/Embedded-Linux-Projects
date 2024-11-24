INCLUDES=-I../include
FLAGS=-Wall

part3: part3.c
	gcc $(FLAGS) $(INCLUDES) -o part3 part3.c ../physical.c -lm -pthread

.PHONY:	clean

clean:
	rm -f part3
