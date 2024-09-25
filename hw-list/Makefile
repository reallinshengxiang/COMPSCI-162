EXECUTABLES=pthread words lwords pwords
CC=gcc
CFLAGS=-g3 -pthread -Wall -std=gnu99
LDFLAGS=-pthread

.PHONY: all clean

all: $(EXECUTABLES)

UNAME := $(shell uname -m)
OBJ_SUFFIX := .o
ifeq "$(UNAME)" "arm64"
    OBJ_SUFFIX := _arm.o
endif
ifeq "$(UNAME)" "aarch64"
    OBJ_SUFFIX := _arm.o
endif

pthread: pthread.o
words: words$(OBJ_SUFFIX) word_helpers$(OBJ_SUFFIX) word_count$(OBJ_SUFFIX)
lwords: lwords$(OBJ_SUFFIX) word_count_l.o word_helpers$(OBJ_SUFFIX) list.o debug.o
pwords: pwords.o word_count_p.o word_helpers$(OBJ_SUFFIX) list.o debug.o

$(EXECUTABLES):
	$(CC) $(LDFLAGS) $^ -o $@

word_count_l.o: word_count_l.c
pwords.o: pwords.c
word_count_p.o: word_count_p.c

word_count_l.o:
	$(CC) $(CFLAGS) -DPINTOS_LIST -c $< -o $@

pwords.o word_count_p.o:
	$(CC) $(CFLAGS) -DPINTOS_LIST -DPTHREADS -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.ONESHELL:
clean:
	tmp_dir=`mktemp -d`
	cp words.o lwords.o word_count.o word_helpers.o lwords_arm.o word_count_arm.o word_helpers_arm.o words_arm.o $$tmp_dir
	rm -f $(EXECUTABLES) *.o
	cp $${tmp_dir}/*.o ./
	rm -r $$tmp_dir
