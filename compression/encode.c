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
        "   Compresses files using the LZ78 compression algorithm.\n"
        "   Compressed files are decompressed with the corresponding decoder.\n"
        "\n"
        "USAGE\n"
        "   ./encode [-vh] [-i input] [-o output]\n"
        "\n"
        "OPTIONS\n"
        "   -v          Display compression statistics\n"
        "   -i input    Specify input to compress (stdin by default)\n"
        "   -o output   Specify output of compressed input (stdout by default)\n"
        "   -h          Display program help and usage\n");

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

    struct stat stats; // Declare struct to store file information
    fstat(input, &stats); // Get status of input file
    FileHeader out; // Declare struct for FileHeader
    out.magic = MAGIC; // Assign MAGIC to out struct
    out.protection = stats.st_mode; // Assign PROTECTION to out struct
    fchmod(output, out.protection); // Set permissions to same as the input file
    write_header(output, &out); // Write the FileHeader to the beginning of output

    // Lines 90-119 based on pseudocode from Prof. Darrell Long
    TrieNode *root = trie_create();
    TrieNode *curr_node = root;
    TrieNode *prev_node = NULL;
    uint8_t curr_sym = 0;
    uint8_t prev_sym = 0;
    int next_code = START_CODE;
    while (read_sym(input, &curr_sym) == true) {
        TrieNode *next_node = trie_step(curr_node, curr_sym);
        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            write_pair(output, curr_node->code, curr_sym, bit_len(next_code));
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code = next_code + 1;
        }
        if (next_code == MAX_CODE) {
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }
        prev_sym = curr_sym;
    }
    if (curr_node != root) {
        write_pair(output, prev_node->code, prev_sym, bit_len(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }
    write_pair(output, STOP_CODE, 0, bit_len(next_code));
    flush_pairs(output);

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
