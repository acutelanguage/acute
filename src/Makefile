CC=clang
CFLAGS=-Wall -Wextra -Werror -Os -I/usr/local/include
OBJECTS=deque.o refuse.o hash.o
LIB=libacute.a

all: lib

lib: $(OBJECTS)
	$(AR) rcs $(LIB) $(OBJECTS)

clean:
	$(RM) $(OBJECTS) $(LIB)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
