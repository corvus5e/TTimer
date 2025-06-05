src=main.c textures.c render.c
target=ttimer
main:
	gcc -Wall $(src) -lncurses -o $(target)
