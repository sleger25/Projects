#Assignment 2: Http Server


## Short Description
This program contains the `httpserver` program. This program allows the user to `get` or `put` data to and from various files, emulating a computer system with a seperate memory and interpreter.

## Files
The repository contains the files
- httpserver.c
- asgn2_helper_funcs.a
- asgn2_helper_funcs.h
- Makefile
- README.md

## Building and Cleaning
To build all required files, simply run `make` or `make all` in terminal. This creates the httpserver executable file. To clean the directory, run `make clean`. This removes the executable and object files. `make format` also clang-formats all c code.

## Running
First, start the server with a desired port number. `./httpserver <port_num>`

To get data, pipe the following command in: `GET /<location> HTTP/1.1\r\n\r\n`
`Get` is intuitive, you simply run the command with the file name you would like information from, and all the data in the file is presented in the terminal.

To put data, run `PUT /<location> HTTP/1.1\r\nContent-Length: value\r\n\r\n<content>`.
Put requires the user pass in a content length, in addition to the text they would like to put into the file. The program will attempt to write `content_length` bytes into the file, but will write as many as possible if the length is longer than the length of the provided data.

## Errors
If an unknown command is given as a parameter, the program will print out a help message. If the data is bad or the input is invalid, corresponding errors are sent.

