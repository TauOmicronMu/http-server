.PHONY: all clean

#compiler c
CC = gcc
CFLAGS = -D_POSIX_SOURCE -Wall -Werror -pedantic -std=c99 -D_GNU_SOURCE -pthread -g
LFLAGS = -pthread

###################################

#c files
SOURCE = $(wildcard *.c) #all .c files
HEADER = $(SOURCE:.c=.h)
OBJS = $(SOURCE:.c=.o)
TARGETS = server
DEPS = http.o freer.o

###################################

#standard rules
all: $(TARGETS)

server: server.o $(DEPS)
	$(CC) $(LFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGETS)

###################################

#overwrite suffix rules to enforce our rules
.SUFFIXES:

%.o: %.c
	$(CC) -c $(CFLAGS) $<

#EOF
