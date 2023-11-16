# Assignment 6: Lempel-Ziv Compression

## Short Description
This program contains two program files: an encoder and a decoder. There are three modules in the repository (trie, word, io) that have relevant functions. This program conducts Lempel-Ziv 78 compression, which uses prefix trees and word tables to create codes that minimize byte usage.

## Files
The repository contains the files
- encode.c
- decode.c
- trie.c, trie.h: prefix tree module
- word.c, word.h: word table module
- io.c, io.h: input/output module
- code.h, endian.h: various helper functions
- Makefile

## Building and Cleaning
To build all required files, simply run `make` or `make all` in terminal. This creates the encode and decode executable files and associated object files. You can also use `make` followed by the target you would like to make (encode, decode) to make only that executable. To clean the directory, run `make clean`. This removes the executable and object files. `Make format` also clang-formats all c code. `Make scan-build` can be run to run scan build during compilation, checking for additional errors. Note: scan-build reports a false positive: a potential memory leak found in word.c this is not a threat because the function with the leak checks to see the value of the word object before freeing it, ensuring it only frees memory that is used.

## Running
To run the code, first run `./encode`. Include input (for compression) and output (to send the compressed file). The input is stdin by default and the output is stdout. These can be specified using -i and -o arguments. Lastly, run `./decode`. Once again, make sure to specify the input and the output. A text file can be encoded and decoded with the following statement: `./encode -i "filename.txt" | ./decode` This encodes the text file and pipes the data into the decoder.

## Errors
If an unknown argument is given as a parameter, the program will print out a help message. If the data is bad or the input is invalid, corresponding errors are sent.

