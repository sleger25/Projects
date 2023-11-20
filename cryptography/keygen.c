#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

// header files
#include "numtheory.h"
#include "randstate.h"
#include "ss.h"

#define OPTIONS "hvb:i:n:d:s:" //these are our argument options

//here we initialize all flag booleans
bool v_flag = false;

void print_help(void) { // helper function for printing help
    fprintf(stderr,

        "SYNOPSIS\n"
        "Generates an SS public/private key pair.\n"
        "\n"
        "USAGE\n"
        "./keygen [OPTIONS]\n"
        "\n"
        "OPTIONS\n"
        "-h              Display program help and usage.\n"
        "-v              Display verbose program output.\n"
        "-b bits         Minimum bits needed for public key n (default: 256).\n"
        "-i iterations   Miller-Rabin iterations for testing primes (default: 50).\n"
        "-n pbfile       Public key file (default: ss.pub).\n"
        "-d pvfile       Private key file (default: ss.priv).\n"
        "-s seed         Random seed for testing.\n");

    return;
}

void change_v_flag(bool *v_flag) { //these are the flag-flipper funcions
    *v_flag = true;
}

int main(int argc, char **argv) {
    int opt = 0;
    char *pbfile = "ss.pub", *pvfile = "ss.priv", *pnbits = NULL, *piters = NULL, *pseed = NULL;
    uint64_t nbits = 256;
    uint64_t iters = 50;
    uint64_t seed = time(NULL);

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) { //while loop to parse arguments
        switch (opt) {
        case 'h': print_help(); break;
        case 'v':
            change_v_flag(&v_flag); //flag is flipped if argument is given
            break;
        case 'b':
            pnbits = optarg; //set char pointer to optarg
            nbits = strtoul(pnbits, NULL, 10);
            break;
        case 'i':
            piters = optarg;
            iters = strtoul(piters, NULL, 10);
            break;
        case 'n': pbfile = optarg; break;
        case 'd': pvfile = optarg; break;
        case 's':
            pseed = optarg;
            seed = strtoul(pseed, NULL, 10);
            break;
        default:
            print_help();
            return 1;
            break;
        }
    }

    FILE *pub_file, *priv_file;

    // Create and open "ss.pub" for writing
    pub_file = fopen(pbfile, "w");
    if (pub_file == NULL) {
        perror("Error opening public key file");
        return 1;
    }

    // Create and open "ss.priv" for writing
    priv_file = fopen(pvfile, "w");
    if (priv_file == NULL) {
        perror("Error opening private key file");
        return 1;
    }

    // Set the permissions of "ss.priv" to 0600
    int priv_fd = fileno(priv_file);

    fchmod(priv_fd, S_IRUSR | S_IWUSR);

    randstate_init(seed);

    mpz_t p, q, n, d, pq;
    mpz_inits(p, q, n, d, pq, NULL);
    ss_make_pub(p, q, n, nbits, iters);
    ss_make_priv(d, pq, p, q);

    char *username = getenv("USER");
    if (username == NULL) {
        printf("Could not determine username.\n");
    }

    ss_write_pub(n, username, pub_file);
    ss_write_priv(pq, d, priv_file);

    //if verbose output is enabled
    if (v_flag == true) { //if -v is an argument, enable verbose output
        printf("user = %s\n", username);
        gmp_printf("p  (%d bits) = %Zd\n", mpz_sizeinbase(p, 2), p);
        gmp_printf("q  (%d bits) = %Zd\n", mpz_sizeinbase(q, 2), q);
        gmp_printf("n  (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("pq (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
        gmp_printf("d  (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    // Close the files
    fclose(pub_file);
    fclose(priv_file);

    randstate_clear();
    mpz_clears(p, q, n, d, pq, NULL);
    return 0;
}
