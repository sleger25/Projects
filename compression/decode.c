#include <math.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Header Files
#include "code.h"
#include "io.h"
#include "word.h"
#include "trie.h"

#define OPTIONS "vi:o:h" // These are our argument options

// Here we initialize all flag booleans
bool v_flag = false;

// Helper function for printing help
void print_help(void) {
    fprintf(stderr,

        "SYNOPSIS\n"
        "   Decompresses files with the LZ78 decompression algorithm.\n"
        "   Used with files compressed with the corresponding encoder.\n"
        "\n"
        "USAGE\n"
        "   ./decode [-vh] [-i input] [-o output]\n"
        "\n"
        "OPTIONS\n"
        "   -v          Display decompression statistics\n"
        "   -i input    Specify input to decompress (stdin by default)\n"
        "   -o output   Specify output of decompressed input (stdout by default)\n"
        "   -h          Display program usage\n");

    return;
}

// Helper function for calculating bit length
int bit_len(int bits) {
    return (int) log2(bits) + 1;
}

int main(int argc, char **argv) {
    int opt = 0;
    int input = STDIN_FILENO; // Set input to STDIN file descriptor
    int output = STDOUT_FILENO; // Set output to STDOUT file descriptor

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) { // While loop to parse arguments
        switch (opt) {
        case 'h': print_help(); return 0;
        case 'v':
            v_flag = true; // Boolean flipped if v is an argument
            break;
        case 'i':
            // Open input file for read-only
            input = open(optarg, O_RDONLY);
            if (input == -1) {
                perror("Error opening input file.");
                return 1;
            }
            break;
        case 'o':
            // Open output file for write only, create if doesn't exist and truncate if does
            output = open(optarg, O_WRONLY | O_CREAT | O_TRUNC);
            if (output == -1) {
                perror("Error opening output file.");
                return 1;
            }
            break;
        default:
            print_help();
            return 1;
            break;
        }
    }

    FileHeader out; // Declare FileHeader variable
    read_header(input, &out); // Read header information into variable

    // Verify the magic number
    if (out.magic != MAGIC) {
        fprintf(stderr, "Bad magic number!\n");
        exit(1);
    }

    fchmod(output, out.protection); // Set permissions to same as the input file

    // Lines 93-106 based on pseudocode from Prof. Darrell Long
    WordTable *table = wt_create();
    uint8_t curr_sym = 0;
    uint16_t curr_code = 0;
    int next_code = START_CODE;
    while (read_pair(input, &curr_code, &curr_sym, bit_len(next_code)) == true) {
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        write_word(output, table[next_code]);
        next_code = next_code + 1;
        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }
    flush_words(output);

    // Check if verbose output enabled
    if (v_flag == true) {
        printf("Compressed file size: %lu bytes\n", total_bits / 8);
        printf("Uncompressed file size: %lu bytes\n", total_syms);
        printf("Space saving: %2.2f%%\n", 100 * (1 - ((float) (total_bits / 8) / total_syms)));
    }

    close(input);
    close(output);

    return 0;
}
