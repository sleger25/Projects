SHELL := /bin/sh

CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic -gdwarf-4 
LDFLAGS = -lm

ENCODE_OBJS = encode.o trie.o word.o io.o
DECODE_OBJS = decode.o trie.o word.o io.o

#all: encode
#$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

all: encode decode

encode: $(ENCODE_OBJS)
	$(CC) -o encode $(ENCODE_OBJS) $(LDFLAGS)

decode: $(DECODE_OBJS)
	$(CC) -o decode $(DECODE_OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f encode $(ENCODE_OBJS) decode $(DECODE_OBJS)

scan-build: clean
	scan-build --use-cc=$(CC) make

format:
	clang-format -i -style=file *.[ch]
