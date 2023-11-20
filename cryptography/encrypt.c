#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

// header files
#include "numtheory.h"
#include "randstate.h"
#include "ss.h"

#define OPTIONS "hvi:o:n:" //these are our argument options

//here we initialize all flag booleans
bool v_flag = false;
bool i_flag = false;
bool o_flag = false;

void print_help(void) { // helper function for printing help
    fprintf(stderr,

        "SYNOPSIS\n"
        "   Encrypts data using SS encryption.\n"
        "   Encrypted data is decrypted by the decrypt program.\n"
        "\n"
        "USAGE\n"
        "   ./encrypt [OPTIONS]\n"
        "\n"
        "OPTIONS\n"
        "   -h              Display program help and usage.\n"
        "   -v              Display verbose program output.\n"
        "   -i infile       Input file of data to encrypt (default: stdin).\n"
        "   -o outfile      Output file for encrypted data (default: stdout).\n"
        "   -n pbfile       Public key file (default: ss.pub).\n");

    return;
}

void change_v_flag(bool *v_flag) { //these are the flag-flipper functions
    *v_flag = true;
}

void change_i_flag(bool *i_flag) {
    *i_flag = true;
}

void change_o_flag(bool *o_flag) {
    *o_flag = true;
}

int main(int argc, char **argv) {
    int opt = 0;
    FILE *input = stdin;
    FILE *output = stdout;
    char *pbfile = "ss.pub";

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) { //while loop to parse arguments
        switch (opt) {
        case 'h': print_help(); return 0;
        case 'v':
            change_v_flag(&v_flag); //flag is flipped if argument is given
            break;
        case 'i':
            change_i_flag(&i_flag);
            input = fopen(optarg, "r");
            if (input == NULL) {
                perror("Error opening input file");
                return 1;
            }
            break;
        case 'o':
            change_o_flag(&o_flag);
            output = fopen(optarg, "w");
            if (output == NULL) {
                perror("Error opening output file");
                return 1;
            }
            break;
        case 'n': pbfile = optarg; break;
        default:
            print_help();
            return 1;
            break;
        }
    }

    mpz_t n;
    mpz_init(n);

    char *username = malloc(LOGIN_NAME_MAX + 1);
    if (username == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Create and open "ss.pub" for writing
    FILE *pub_file;
    pub_file = fopen(pbfile, "r");

    ss_read_pub(n, username, pub_file);

    //check verbose output
    if (v_flag == true) { //if -v is an argument, enable verbose output
        printf("user = %s\n", username);
        gmp_printf("n  (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
    }

    ss_encrypt_file(input, output, n);

    fclose(pub_file);

    if (i_flag == true) {
        fclose(input);
    }

    if (o_flag == true) {
        fclose(output);
    }

    mpz_clear(n);
    free(username);

    // Close the files

    return 0;
}
