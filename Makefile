src=main.c textures.c ncurses_io.c
target=ttimer
main:
	gcc -Wall $(src) -lncurses -o $(target)
