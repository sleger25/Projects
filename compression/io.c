#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Header Files
#include "code.h"
#include "endian.h"
#include "io.h"
#include "word.h"

uint64_t total_syms = 0; // To count the symbols processed.
uint64_t total_bits = 0; // To count the bits processed.

static uint8_t pair_buffer[BLOCK] = { 0 }; // Global buffer used with syms and words
static int pb_index = 0; // Index for pair_buffer

static uint8_t buffer[BLOCK] = { 0 }; // Global buffer for read_pair write_pair.
static int b_index = 0; // Index for buffer

static int n_1 = -1; // Represents EOF/-1

uint8_t read_bit(uint8_t *buf, uint64_t bit) {
    // Calculate the index in buf that contains the bit
    // Calculate a bit mask using mod
    // Set the bit in buf by OR-ing it with the mask
    return buf[bit / 8] |= (1 << bit % 8);
}

uint8_t write_bit(uint8_t *buf, uint64_t bit) {
    // Calculate the index in buf that contains the bit
    // Calculate position of bit using mod
    // Returns value by shifting to the right and AND-ing
    return (buf[bit / 8] >> (bit % 8)) & 1;
}

uint8_t clear_bit(uint8_t *buf, uint64_t bit) {
    // Calculate the index in buf that contains the bit
    // Calculate a bit mask using mod
    // Clear the bit in buf by AND-ing it with the negated mask
    return buf[bit / 8] &= ~(1 << bit % 8);
}

//
// Read up to to_read bytes from infile and store them in buf. Return the number of bytes actually
// read.
//
// Since read() may not read in as many bytes as you asked for, this function should continuously
// call read() and attempt to read as many bytes as it has not yet read. For instance, if to_read is
// 100 and the first read() call only reads 20 bytes, it should attempt to read 80 bytes the next
// time it calls read().
//
int read_bytes(int infile, uint8_t *buf, int to_read) {
    int all_bytes_read = 0; // Initialize value to return

    while (1) { // Begin infinite loop
        int bytes_read = read(infile, buf, to_read); // Read bytes in

        if (bytes_read == n_1) { // If bytes read is -1, an error occurred
            fprintf(stderr, "Failed to read bytes.\n");
            exit(1);
        }

        buf += bytes_read; // Advance buffer by bytes read
        all_bytes_read += bytes_read; // Keeps track of all bytes read

        if (all_bytes_read == to_read) { // Check if all data has been read
            break;
        } else if (bytes_read == 0) { // Check if there is no more data to read
            break;
        }
    }
    return all_bytes_read;
}

//
// Write up to to_write bytes from buf into outfile. Return the number of bytes actually written.
//
// Similarly to read_bytes, this function will need to call write() in a loop to ensure that it
// writes as many bytes as possible.
//
int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int all_bytes_written = 0; // Initialize value to return

    while (true) { // Begin infinite loop
        int bytes_written = write(outfile, buf, to_write); // Write bytes out

        if (bytes_written == n_1) { // If bytes written is -1, an error occurred
            fprintf(stderr, "Failed to write bytes.\n");
            exit(1);
        }

        buf += bytes_written; // Advance buffer by bytes written
        all_bytes_written += bytes_written; // Keep track of all bytes written

        if (all_bytes_written == to_write) { // Check if all data has been written
            break;
        } else if (bytes_written == 0) { // Check if there is no more data to write
            break;
        }
    }
    return all_bytes_written;
}

//
// Read a file header from infile into *header.
//
// This function need not create any additional buffer to store the contents of the file header.
// File headers, like any struct or any value in C, are just represented by bytes in memory which
// means you can use read_bytes to read however many bytes the header consumes (use sizeof!) into
// *header.
//
// Since we decided that the canonical byte order for our headers is little-endian, this function
// will need to swap the byte order of both the header fields if it is run on a big-endian system.
// For example, here is how the 4 bytes of the magic number will look when written to the file:
//
// +------+------+------+------+
// | 0xAC | 0xBA | 0xAD | 0xBA |
// +------+------+------+------+
//
// A big-endian computer would interpret those bytes as the 4-byte number 0xACBAADBA, which is
// not what we want, so you would have to change the order of those bytes in memory. A little-endian
// computer will interpret that as 0xBAADBAAC.
//
// This function should also make sure the magic number is correct. Since it has no return value you
// may call assert() to do that, or print out an error message and exit the program, or use some
// other way to report the error.
//
void read_header(int infile, FileHeader *header) {
    // Read in the header data from the input file directly into the header struct
    read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));

    // Swap the byte order of the header fields if the system is big endian
    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    total_bits += 48;
}

//
// Write a file header from *header to outfile. Like above, this function should swap the byte order
// of the header's two fields if necessary.
//
void write_header(int outfile, FileHeader *header) {
    // Swap the byte order of the header fields if the system is big endian
    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    // Write the header data to the output file directly from the header struct
    write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader));
    total_bits += 48;
}

//
// Read one symbol from infile into *sym. Return true if a symbol was successfully read, false
// otherwise.
//
// Reading one symbol at a time is slow, so this function will need to maintain a global buffer
// (an array) of BLOCK bytes. Most calls will only need to read a symbol out of that buffer, and
// then update some counter so that the function knows what position in the buffer it is at. If
// there are no more bytes in the buffer for it to return, it will have to call read_bytes to refill
// the buffer with fresh data. If this call fails then you cannot read a symbol and should return
// false.
//
bool read_sym(int infile, uint8_t *sym) {
    if (pb_index == n_1) { // If buffer index is -1, EOF is reached
        return false;
    }

    if (pb_index % BLOCK == 0) { // Check if buffer is empty and needs to be refilled
        int bytes_read = read_bytes(infile, pair_buffer, BLOCK);
        if (bytes_read < BLOCK) {
            n_1 = pb_index + bytes_read; // Keeps track of the end of file
        }
    }

    *sym = pair_buffer[pb_index % BLOCK]; // Point sym to correct index in buffer
    pb_index += 1;
    total_syms += 1;

    return true;
}

//
// Write a pair -- bitlen bits of code, followed by all 8 bits of sym -- to outfile.
//
// This function should also use a buffer. It writes into individual bits in the buffer, starting
// with the least significant bit of the first byte, until the most significant bit of the first
// byte, and then the least significant bit of the second byte, and so on. You will need bitwise
// arithmetic to manipulate individual bits within your buffer -- consult asgn3/set.c if you don't
// remember how to do this.
//
// The first bit of code to be written is the least significant bit, and the same holds for sym.
//
// This function will need to track which *bit* in the buffer will be written to next. If it ever
// reaches the end of the buffer it needs to write out the contents of the buffer to outfile; you
// may use flush_pairs to do this.
//
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    for (int i = 0; i < bitlen; i++) { // Loop through bitlen bits of code
        uint16_t code_bit = ((code >> (i % 16)) & 1); // Extracts the i-th bit
        if (code_bit == 1) {
            read_bit(buffer, b_index); // Next bit to be written is 1
        } else {
            clear_bit(buffer, b_index); // Next bit to be written is 0
        }

        b_index += 1;

        if (b_index == (8 * BLOCK)) { // Check if end of block has been reached
            write_bytes(outfile, buffer, BLOCK); // Write out the buffer
            total_bits += b_index;
            b_index = 0; // Reset the buffer index
        }
    }

    for (int i = 0; i < 8; i++) { // Loop through bits of sym
        uint8_t sym_bit = ((sym >> (i % 8)) & 1); // Extracts the i-th bit
        if (sym_bit == 1) {
            read_bit(buffer, b_index); // Next bit to be written is 1
        } else {
            clear_bit(buffer, b_index); // Next bit to be written is 0
        }

        b_index += 1;

        if (b_index == 8 * BLOCK) { // Check if end of block has been reached
            write_bytes(outfile, buffer, BLOCK); // Write out the buffer
            total_bits += b_index;
            b_index = 0; // Reset the buffer index
        }
    }
}

//
// Write any pairs that are in write_pair's buffer but haven't been written yet to outfile.
//
// This function will need to be called at the end of encode since otherwise those pairs would never
// be written. You don't have to, but it would be easy to make this function also work when called
// by write_pair, since otherwise you would write essentially the same code in two places.
//
// If not all bits of the last byte in your buffer have been written to, make sure that the
// unwritten bits are set to zero. An easy way to do this is by zeroing the entire buffer after
// flushing it every time.
//
uint64_t to_bytes(uint64_t bits) {
    if (bits % 8) { // Check if there is a partial byte
        return (bits / 8) + 1; // Calculates storage with partial byte
    } else {
        return (bits / 8); // Calculates storage without partial byte
    }
}

void flush_pairs(int outfile) {
    total_bits += b_index;
    write_bytes(outfile, buffer, to_bytes(b_index));
}

//
// Read bitlen bits of a code into *code, and then a full 8-bit symbol into *sym, from infile.
// Return true if the complete pair was read and false otherwise.
//
// Like write_pair, this function must read the least significant bit of each input byte first, and
// will store those bits into the LSB of *code and of *sym first.
//
// It may be useful to write a helper function that reads a single bit from a file using a buffer.
//
bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    *code = 0;
    *sym = 0;
    for (int i = 0; i < bitlen; i++) { // Loop through bitlen bits of code
        if (b_index == 0) { // Check if buffer is empty
            read_bytes(infile, buffer, BLOCK); // Read bytes into the buffer
        }

        *code |= write_bit(buffer, b_index) << i; // Set the i-th bit of code
        total_bits += 1;
        b_index = (b_index + 1) % (BLOCK * 8); // Update the buffer index
    }

    for (int i = 0; i < 8; i++) { // Loop through bits of sym
        if (b_index == 0) { // Check if buffer is empty
            read_bytes(infile, buffer, BLOCK); // Read bytes into the buffer
        }

        *sym |= write_bit(buffer, b_index) << i; // Set the i-th bit of sym
        total_bits += 1;
        b_index = (b_index + 1) % (BLOCK * 8); // Update the buffer index
    }

    if (*code != STOP_CODE) { // Check if there are pairs left to read
        return true;
    } else {
        return false;
    }
}

//
// Write every symbol from w into outfile.
//
// These symbols should also be buffered and the buffer flushed whenever necessary (note you will
// likely sometimes fill up your buffer in the middle of writing a word, so you cannot only check
// that the buffer is full at the end of this function).
//
void write_word(int outfile, Word *w) {
    for (uint32_t i = 0; i < w->len; i++) { // Loop through each symbol in word
        if (pb_index == BLOCK) { // Check if buffer is full
            write_bytes(outfile, pair_buffer, BLOCK); // Write out the buffer
            total_syms += pb_index;
            pb_index = 0; // Reset the buffer index
        }
        // Item at word index i is written to corresponding word buffer index
        pair_buffer[pb_index] = w->syms[i];
        pb_index += 1;
    }

    if (pb_index > 0) { // Check if there are still symbols in buffer
        write_bytes(outfile, pair_buffer, pb_index);
        total_syms += pb_index;
        pb_index = 0;
    }
}

//
// Write any unwritten word symbols from the buffer used by write_word to outfile.
//
// Similarly to flush_pairs, this function must be called at the end of decode since otherwise you
// would have symbols remaining in the buffer that were never written.
//
void flush_words(int outfile) {
    write_bytes(outfile, pair_buffer, pb_index);
}
