src=main.c
target=ttimer
main:
	gcc -Wall $(src) -lncurses -o $(target)
