build_dir=build
target=ttimer
src=src/main.c \
    src/timer/timer.c \
    src/ui/textures.c \
    src/ui/settings_view.c \
    src/ui/graph_view.c \
    src/ui/timer_view.c \
    src/db/db_sqlite.c \
    src/HomeTUI/ncurses_io.c \
    src/HomeTUI/home_tui.c \

main: prepare $(build_dir)/libsqlite3.a
	gcc -std=c11 -g -Wall -I ./src/ $(src) -lncurses -lm -L$(build_dir) -lsqlite3 -o $(build_dir)/$(target)

prepare:
	mkdir -p build
	mkdir -p data

 $(build_dir)/libsqlite3.a: $(build_dir)/sqlite3.o
	ar rcs $(build_dir)/libsqlite3.a $(build_dir)/sqlite3.o

 $(build_dir)/sqlite3.o:
	gcc -c -std=c11 -Wall src/db/sqlite3/sqlite3.c -o $(build_dir)/sqlite3.o

profile: $(build_dir)/libsqlite3.a # for gprof
	gcc -std=c11 -Wall $(src) -pg -lncurses -lpthread -L$(build_dir) -lsqlite3

run:
	./$(build_dir)/$(target) 2> data/std_err.log
