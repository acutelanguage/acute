CC=clang
CFLAGS=-Wall -Wextra -Werror -Os -I/usr/local/include -I../src
OBJECTS=refuse_test.o hash_test.o tests.o
BIN=tests
LIBS=../src/libacute.a -L/usr/local/lib -lcheck

all: test

clean:
	$(RM) $(OBJECTS) $(LIB) tests

test: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(BIN) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
