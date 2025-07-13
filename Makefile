src=main.c timer.c textures.c ncurses_io.c db_sqlite.c
target=ttimer

main: libsqlite3.a
	gcc -std=c11 -Wall $(src) -lncurses -lm -L. -lsqlite3 -o $(target)

sqlite3.o:
	gcc -c -std=c11 -Wall sqlite3/sqlite3.c -o sqlite3.o

libsqlite3.a: sqlite3.o
	ar rcs libsqlite3.a sqlite3.o

profile: libsqlite3.a # for gprof
	gcc -std=c11 -Wall $(src) -pg -lncurses -lpthread -L. -lsqlite3

run:
	./$(target) 2>std_err.log
