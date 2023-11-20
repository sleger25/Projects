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
        "   Decrypts data using SS decryption.\n"
        "   Encrypted data is encrypted by the encrypt program.\n"
        "\n"
        "USAGE\n"
        "   ./decrypt [OPTIONS]\n"
        "\n"
        "OPTIONS\n"
        "   -h              Display program help and usage.\n"
        "   -v              Display verbose program output.\n"
        "   -i infile       Input file of data to decrypt (default: stdin).\n"
        "   -o outfile      Output file for decrypted data (default: stdout).\n"
        "   -n pvfile       Private key file (default: ss.priv).\n");

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
    char *pvfile = "ss.priv";

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
        case 'n': pvfile = optarg; break;
        default:
            print_help();
            return 1;
            break;
        }
    }

    mpz_t pq, d;
    mpz_inits(pq, d, NULL);

    // Create and open "ss.priv" for writing
    FILE *priv_file;
    priv_file = fopen(pvfile, "r");

    ss_read_priv(pq, d, priv_file);

    //check verbose output
    if (v_flag == true) { //if -v is an argument, enable verbose output
        gmp_printf("pq  (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
        gmp_printf("d  (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }
    ss_decrypt_file(input, output, d, pq);
    fclose(priv_file);

    if (i_flag == true) {
        fclose(input);
    }

    if (o_flag == true) {
        fclose(output);
    }

    mpz_clears(pq, d, NULL);

    // Close the files

    return 0;
}
