CC:=clang
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
	$(CC) -std=c11 -Wall \
		-D_POSIX_C_SOURCE=199309L \
		-I ./src/ $(src) \
		$(shell ncursesw6-config --cflags --libs) \
		-lm -lsqlite3 \
		-L$(build_dir) \
		-o $(build_dir)/$(target)

prepare:
	mkdir -p build
	mkdir -p data

 $(build_dir)/libsqlite3.a: $(build_dir)/sqlite3.o
	ar rcs $(build_dir)/libsqlite3.a $(build_dir)/sqlite3.o

 $(build_dir)/sqlite3.o:
	$(CC) -c -std=c11 -Wall src/db/sqlite3/sqlite3.c -o $(build_dir)/sqlite3.o

profile: $(build_dir)/libsqlite3.a # for gprof
	$(CC) -std=c11 -Wall $(src) -pg -lncurses -lpthread -L$(build_dir) -lsqlite3

clean:
	rm -rdf $(build_dir)

run:
	./$(build_dir)/$(target) 2> data/std_err.log
