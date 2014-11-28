CC=gcc
CFLAGS=-O2 -Wall -pthread
LIBS=-lmigration

default: migrationbench

migrationbench: migrationbench.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -rf *.o
	rm -rf migrationbench
