.POSIX:

CC		= gcc
CFLAGS	= -std=c99 -O3 -fPIC -Wall
LDFLAGS	= -s
LDLIBS	= -lncurses

.PHONY: puissance4 clean

puissance4: puissance4.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(LDLIBS) -o $@ $+

clean:
	rm -f puissance4
