all: demo background profile main util
profile:
	touch profile
background: background.o util.o
	gcc background.o util.o -o background
demo: main.o util.o
	gcc main.o util.o -o demo
util: util.c
	gcc -c util.c
clean:
	rm *.o *_pipe demo background
main: main.c
	gcc -c main.c
backend: background.c
	gcc -c background.c
pipe:
	rm *_pipe
