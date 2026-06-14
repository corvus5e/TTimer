CC:=clang
build_dir=build
target=ttimer
hometui_dir=src/HomeTUI
hometui_lib=$(hometui_dir)/bin/libhome_tui.a

src=src/main.c \
    src/timer/timer.c \
    src/ui/textures.c \
    src/ui/settings_view.c \
    src/ui/graph_view.c \
    src/ui/timer_view.c \
    src/db/db_sqlite.c \

main: prepare $(build_dir)/libsqlite3.a $(hometui_lib)
	$(CC) -std=c11 -Wall \
		-D_POSIX_C_SOURCE=199309L \
		-DUSE_UTF8 \
		-I ./src/ $(src) \
		$(shell ncursesw6-config --cflags --libs) \
		-lm -lsqlite3 \
		-L$(build_dir) \
		-L$(hometui_dir)/bin -lhome_tui \
		-o $(build_dir)/$(target)

prepare:
	mkdir -p build
	mkdir -p data

$(hometui_lib):
	$(MAKE) -C $(hometui_dir) UTF-8_lib CC=$(CC)

 $(build_dir)/libsqlite3.a: $(build_dir)/sqlite3.o
	ar rcs $(build_dir)/libsqlite3.a $(build_dir)/sqlite3.o

 $(build_dir)/sqlite3.o:
	$(CC) -c -std=c11 -Wall src/db/sqlite3/sqlite3.c -o $(build_dir)/sqlite3.o

profile: $(build_dir)/libsqlite3.a $(hometui_lib) # for gprof
	$(CC) -std=c11 -Wall -DUSE_UTF8 $(src) -pg -lncurses -lpthread -L$(build_dir) -lsqlite3 -L$(hometui_dir)/bin -lhome_tui

clean:
	rm -rdf $(build_dir)
	$(MAKE) -C $(hometui_dir) clean

run:
	./$(build_dir)/$(target) 2> data/std_err.log
