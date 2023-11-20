#include <stdio.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "randstate.h"
#include "numtheory.h"

//
// Generates the components for a new SS key.
//
// Provides:
//  p:  first prime
//  q: second prime
//  n: public modulus/exponent
//
// Requires:
//  nbits: minimum # of bits in n
//  iters: iterations of Miller-Rabin to use for primality check
//  all mpz_t arguments to be initialized
//
void ss_make_pub(mpz_t p, mpz_t q, mpz_t n, uint64_t nbits, uint64_t iters) {
    uint64_t p_range = ((2 * nbits) / 5) - (nbits / 5) + 1;
    uint64_t p_bits = (uint64_t) (random() % p_range) + (nbits / 5);
    make_prime(p, p_bits, iters);
    make_prime(q, p_bits, iters);

    // compute p-1
    mpz_t p_1;
    mpz_init(p_1);
    mpz_sub_ui(p_1, p, 1);

    // compute q-1
    mpz_t q_1;
    mpz_init(q_1);
    mpz_sub_ui(q_1, q, 1);

    // compute the remainder of p modulo q-1
    mpz_t p_remainder;
    mpz_init(p_remainder);
    mpz_mod(p_remainder, p, q_1);

    // compute the remainder of q modulo p-1
    mpz_t q_remainder;
    mpz_init(q_remainder);
    mpz_mod(q_remainder, q, p_1);

    while ((mpz_cmp_ui(p_remainder, 0) == 0) && (mpz_cmp_ui(q_remainder, 0) == 0)) {
        make_prime(p, p_bits, iters);
        make_prime(q, p_bits, iters);
        // compute p-1
        mpz_sub_ui(p_1, p, 1);

        // compute q-1
        mpz_sub_ui(q_1, q, 1);

        // compute the remainder of p modulo q-1
        mpz_mod(p_remainder, p, q_1);

        // compute the remainder of q modulo p-1
        mpz_mod(q_remainder, q, p_1);
    }
    mpz_clears(p_1, q_1, p_remainder, q_remainder, NULL);
    mpz_mul(n, p, p); // n = p * p
    mpz_mul(n, n, q); // n = p * p * q
}

//
// Generates components for a new SS private key.
//
// Provides:
//  d:  private exponent
//  pq: private modulus
//
// Requires:
//  p:  first prime number
//  q: second prime number
//  all mpz_t arguments to be initialized
//
void ss_make_priv(mpz_t d, mpz_t pq, const mpz_t p, const mpz_t q) {
    mpz_t n, gcd_value, lcm_value;
    mpz_inits(n, gcd_value, lcm_value, NULL);

    // compute p-1
    mpz_t p_1;
    mpz_init(p_1);
    mpz_sub_ui(p_1, p, 1);

    // compute q-1
    mpz_t q_1;
    mpz_init(q_1);
    mpz_sub_ui(q_1, q, 1);

    // compute (p-1)(q-1)
    mpz_t pq_1;
    mpz_init(pq_1);
    mpz_mul(pq_1, p_1, q_1);

    gcd(gcd_value, p_1, q_1); // find the gcd

    mpz_fdiv_q(lcm_value, pq_1, gcd_value); // find lcm by doing floor division

    mpz_mul(pq, p, q); // pq = p * q
    mpz_mul(n, pq, p); // n = pq * p

    mod_inverse(d, n, lcm_value); // calculate d

    mpz_clears(n, p_1, q_1, pq_1, gcd_value, lcm_value, NULL);
}

//
// Export SS public key to output stream
//
// Requires:
//  n: public modulus/exponent
//  username: login name of keyholder ($USER)
//  pbfile: open and writable file stream
//

void ss_write_pub(const mpz_t n, const char username[], FILE *pbfile) {
    gmp_fprintf(pbfile, "%Zx\n%s\n", n, username);
}

//
// Export SS private key to output stream
//
// Requires:
//  pq: private modulus
//  d:  private exponent
//  pvfile: open and writable file stream
//

void ss_write_priv(const mpz_t pq, const mpz_t d, FILE *pvfile) {
    gmp_fprintf(pvfile, "%Zx\n%Zx\n", pq, d);
}

//
// Import SS public key from input stream
//
// Provides:
//  n: public modulus
//  username: $USER of the pubkey creator
//
// Requires:
//  pbfile: open and readable file stream
//  username: requires sufficient space
//  all mpz_t arguments to be initialized
//

void ss_read_pub(mpz_t n, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%Zx\n%s\n", n, username);
}

//
// Import SS private key from input stream
//
// Provides:
//  pq: private modulus
//  d:  private exponent
//
// Requires:
//  pvfile: open and readable file stream
//  all mpz_t arguments to be initialized
//

void ss_read_priv(mpz_t pq, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%Zx\n%Zx\n", pq, d);
}

//
// Encrypt number m into number c
//
// Provides:
//  c: encrypted integer
//
// Requires:
//  m: original integer
//  n: public exponent/modulus
//  all mpz_t arguments to be initialized
//
void ss_encrypt(mpz_t c, const mpz_t m, const mpz_t n) {
    pow_mod(c, m, n, n);
}

//
// Encrypt an arbitrary file
//
// Provides:
//  fills outfile with the encrypted contents of infile
//
// Requires:
//  infile: open and readable file stream
//  outfile: open and writable file stream
//  n: public exponent and modulus
//

void ss_encrypt_file(FILE *infile, FILE *outfile, const mpz_t n) {

    // Calculate the block size

    mpz_t root;
    mpz_init(root);

    // Compute the integer square root of n and store the result in root
    mpz_sqrt(root, n);

    // Compute the base-2 logarithm of the square root of root
    int k = (mpz_sizeinbase(root, 2) - 1) / 8;

    // Divide the result by 8

    // Allocate memory for the block
    uint8_t *block = (uint8_t *) malloc(k * sizeof(uint8_t));
    if (block == NULL) {
        perror("Failed to allocate memory for block");
        exit(EXIT_FAILURE);
    }

    // Set the first byte of the block to 0xFF
    block[0] = 0xFF;
    mpz_t m, c;
    mpz_inits(m, c, NULL);

    size_t j;
    while ((j = fread(block + 1, sizeof(uint8_t), k - 1, infile)) > 0) {
        // Convert the block to an mpz_t

        mpz_import(m, j + 1, 1, sizeof(uint8_t), 1, 0, block);

        // Encrypt the block

        ss_encrypt(c, m, n);

        // Write the encrypted block to the output file
        gmp_fprintf(outfile, "%Zx\n", c);

        // Free the memory for the mpz_t variables
    }

    // Free the memory for the block
    mpz_clear(root);
    mpz_clears(m, c, NULL);
    free(block);
}

//
// Decrypt number c into number m
//
// Provides:
//  m: decrypted/original integer
//
// Requires:
//  c: encrypted integer
//  d: private exponent
//  pq: private modulus
//  all mpz_t arguments to be initialized
//

void ss_decrypt(mpz_t m, const mpz_t c, const mpz_t d, const mpz_t pq) {
    pow_mod(m, c, d, pq);
}

//
// Decrypt a file back into its original form.
//
// Provides:
//  fills outfile with the unencrypted data from infile
//
// Requires:
//  infile: open and readable file stream to encrypted data
//  outfile: open and writable file stream
//  d: private exponent
//  pq: private modulus
//

void ss_decrypt_file(FILE *infile, FILE *outfile, const mpz_t d, const mpz_t pq) {

    // Calculate the block size
    uint64_t k = (mpz_sizeinbase(pq, 2) - 1) / 8;
    size_t j;

    // Allocate memory for the block
    uint8_t *block = (uint8_t *) malloc(k * sizeof(uint8_t));
    if (block == NULL) {
        perror("Failed to allocate memory for block");
        exit(EXIT_FAILURE);
    }

    // Read in encrypted blocks and decrypt them

    block[0] = 0xFF;
    mpz_t c, m;
    mpz_inits(c, m, NULL);
    while (gmp_fscanf(infile, "%Zx \n", c) != -1) {

        ss_decrypt(m, c, d, pq);

        // Export the decrypted block to a byte array

        mpz_export(block, &j, 1, sizeof(uint8_t), 1, 0, m);

        // Write the decrypted block to the output file
        fwrite(block + 1, sizeof(uint8_t), j - 1, outfile);
    }
    // Free the memory for the mpz_t variables
    mpz_clears(c, m, NULL);
    // Free the memory for the block and hexstring
    free(block);
}
