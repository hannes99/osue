# @author Hannes Laimer <e11808227@student.tuwien.ac.at>
# Exercise: 1A
# Program name: mydiff

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDFLAGS =

PROGS = mydiff

all: $(PROGS)

$(PROGS): % : %.o
	gcc $(LDFLAGS) -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o $(PROGS)
