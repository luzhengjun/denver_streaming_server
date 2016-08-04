main: main.c streaming.o
	gcc -g -o main streaming.o main.c -L/usr/local/ffmpeg/lib -lavformat -lavutil

streaming.o: streaming.c
	gcc -o streaming.o -c streaming.c
