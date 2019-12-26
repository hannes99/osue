# @author Hannes Laimer <e11808227@student.tuwien.ac.at>
# Exercise: 3
# Program name: arc-set

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDFLAGS = -lrt -pthread

OBJECTS = edge.o logger.o shared.o
PROGS = generator supervisor

.PHONY: all clean
all: $(PROGS)

generator: $(OBJECTS) generator.o
	gcc $(LDFLAGS) -o $@ $^

supervisor: $(OBJECTS) supervisor.o
	gcc $(LDFLAGS) -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o $(PROGS)
