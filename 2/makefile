# @author Hannes Laimer <e11808227@student.tuwien.ac.at>
# Exercise: 2
# Program name: forksort

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDFLAGS =

OBJECTS = logger.o
PROGS = forksort

.PHONY: all clean
all: $(PROGS)

forksort: $(OBJECTS) forksort.o
	gcc $(LDFLAGS) -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o $(PROGS)
