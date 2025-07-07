src=main.c timer_state.c textures.c ncurses_io.c log.c
target=ttimer
main:
	gcc -std=c11 -Wall $(src) -lncurses -lpthread -o $(target)
