all:
	gcc -Wall -pthread -o exe main.c

run:
	./exe 2
