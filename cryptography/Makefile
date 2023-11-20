SHELL := /bin/sh

CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic `pkg-config --cflags gmp` -gdwarf-4 
LDFLAGS = `pkg-config --libs gmp`

KEYGEN_OBJS = keygen.o numtheory.o ss.o randstate.o
ENCRYPT_OBJS = encrypt.o numtheory.o ss.o randstate.o
DECRYPT_OBJS = decrypt.o numtheory.o ss.o randstate.o

#all: keygen
#$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

all: keygen encrypt decrypt

keygen: $(KEYGEN_OBJS)
	$(CC) -o keygen $(KEYGEN_OBJS) $(LDFLAGS)

encrypt: $(ENCRYPT_OBJS)
	$(CC) -o encrypt $(ENCRYPT_OBJS) $(LDFLAGS)

decrypt: $(DECRYPT_OBJS)
	$(CC) -o decrypt $(DECRYPT_OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f keygen $(KEYGEN_OBJS) encrypt $(ENCRYPT_OBJS) decrypt $(DECRYPT_OBJS)

scan-build: clean
	scan-build --use-cc=$(CC) make

format:
	clang-format -i -style=file *.[ch]

