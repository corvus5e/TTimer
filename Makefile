src=main.c timer.c textures.c draw.c HomeTUI/ncurses_io.c HomeTUI/home_tui.c db_sqlite.c
build_dir=build
target=ttimer

main: prepare libsqlite3.a
	gcc -std=c11 -g -Wall $(src) -lncurses -lm -L$(build_dir) -lsqlite3 -o $(build_dir)/$(target)

prepare:
	mkdir -p build

libsqlite3.a: sqlite3.o
	ar rcs $(build_dir)/libsqlite3.a $(build_dir)/sqlite3.o

sqlite3.o:
	gcc -c -std=c11 -Wall sqlite3/sqlite3.c -o $(build_dir)/sqlite3.o

profile: libsqlite3.a # for gprof
	gcc -std=c11 -Wall $(src) -pg -lncurses -lpthread -L$(build_dir) -lsqlite3

run:
	./$(build_dir)/$(target) 2>std_err.log
