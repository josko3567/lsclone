CC = gcc
CFLAGS = -Wall -O3
TARGETS = lsclone

default: all

all: $(TARGETS)

lsclone: lsclone.c
	$(CC) $(CFLAGS) lsclone.c -o lsclone.out

clean:
	rm -f *.o *~ a.out $(TARGETS)

.c.o:
	$(CC) $(CFLAGS) -c $<