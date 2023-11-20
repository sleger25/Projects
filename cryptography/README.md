# Assignment 5: Public Key Cryptography

## Short Description
This program contains three program files: a key generator, an encryptor, and a decryptor. There are three modules in the repository (randstate, numtheory, ss) that have relevant functions. This program conducts public-key cryptography, which uses pairs of related keys that are computed by one-way mathematical functions.


## Files
The repository contains the files
- keygen.c
- encrypt.c
- decrypt.c
- ss.c, ss.h: cryptography library
- numtheory.c, numtheory.h: math library
- randstate.c, randstate.h: random generator object module
- Makefile

## Building and Cleaning
To build all required files, simply run `make` or `make all` in terminal. This creates the keygen, encrypt, and decrypt executable files and associated object files. You can also use `make` followed by the target you would like to make (keygen, encrypt, decrypt) to make only that executable. To clean the directory, run `make clean`. This removes the executable and object files. `Make format` also clang-formats all c code. `Make scan-build` can be run to run scan build during compilation, checking for additional errors.

## Running
To run the code, first run `./keygen`. This creates the public and private keys and prints them to their respective files. Then run `./encrypt`. Include input (for encyption) and output (to send the encrypted message). The input is stdin by default and the output is stdout. These can be specified using -i and -o arguments. Lastly, run `./decrypt`. Once again, make sure to specify the input and the output. A text file can be encrypted and decrypted with the following statement: `./encrypt -i "filename.txt" | ./decrypt` This encrypts the text file and pipes the data into the decryptor.

## Errors
If an unknown argument is given as a parameter, the program will print out a help message. If the data is bad or the input is invalid, corresponding errors are sent.

