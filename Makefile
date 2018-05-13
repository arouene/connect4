.POSIX:

CC		= gcc
CFLAGS	= -std=c99 -O3 -fPIC -Wall
LDFLAGS	= -s
LDLIBS	= -lncurses

.PHONY: all clean

all: puissance4 puissance4_minimax

puissance4: puissance4.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(LDLIBS) -o $@ $+

puissance4_minimax: puissance4_minimax.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(LDLIBS) -o $@ $+

clean:
	rm -f puissance4 puissance4_minimax
