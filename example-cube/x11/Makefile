all: cube rotate

rotate: rotate.c
	gcc -g -o $@ $^

cube: cube.cpp
	gcc -o $@ $^ -L/usr/X11R6/lib/ -lGL -lGLU -lX11

clean:
	rm -f *.o cube rotate DEADJOE *~
