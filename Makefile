src=main.c textures.c sleep_timer.c
target=ttimer
main:
	echo "Select target: naive|ncurses"
naive:
	gcc -Wall $(src) naive_render.c -o $(target)
ncurses:
	gcc -Wall $(src) ncurses_render.c -lncurses -o $(target)
