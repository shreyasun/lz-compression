CC = clang
CFLAGS = -Wall -Wpedantic -Werror -Wextra -g -gdwarf-4

.PHONY: all clear

all: encode decode

encode: encode.o io.o word.o trie.o
	$(CC) -o $@ $^

decode: decode.o io.o word.o trie.o
	$(CC) -o $@ $^	

clean:
	rm *.o encode decode

format:
	clang-format -i -style=file *.[ch]

scan-build: clean
	scan-build --use-cc=$(CC) make

