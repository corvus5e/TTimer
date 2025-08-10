src=main.c timer.c \
    ui/textures.c ui/settings_view.c ui/graph_view.c ui/timer_view.c \
    db/db_sqlite.c \
    HomeTUI/ncurses_io.c HomeTUI/home_tui.c \

build_dir=build
target=ttimer

main: prepare $(build_dir)/libsqlite3.a
	gcc -std=c11 -g -Wall -I . $(src) -lncurses -lm -L$(build_dir) -lsqlite3 -o $(build_dir)/$(target)

prepare:
	mkdir -p build

 $(build_dir)/libsqlite3.a: $(build_dir)/sqlite3.o
	ar rcs $(build_dir)/libsqlite3.a $(build_dir)/sqlite3.o

 $(build_dir)/sqlite3.o:
	gcc -c -std=c11 -Wall db/sqlite3/sqlite3.c -o $(build_dir)/sqlite3.o

profile: $(build_dir)/libsqlite3.a # for gprof
	gcc -std=c11 -Wall $(src) -pg -lncurses -lpthread -L$(build_dir) -lsqlite3

run:
	./$(build_dir)/$(target) 2>std_err.log
