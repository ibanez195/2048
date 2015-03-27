all:
	gcc -g -Wall src/2048.c -o 2048 -lncurses
clean:
	rm 2048
