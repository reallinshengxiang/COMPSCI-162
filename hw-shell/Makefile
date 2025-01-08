SRCS=shell.c tokenizer.c
EXECUTABLES=shell

CC=gcc
CFLAGS=-g3 -Wall -Werror -std=gnu99 

OBJS=$(SRCS:.c=.o)

all: $(EXECUTABLES)

$(EXECUTABLES): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXECUTABLES) $(OBJS)
