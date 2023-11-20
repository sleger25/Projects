#include <stdio.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdint.h>
#include "randstate.h"
#include <time.h>

void gcd(mpz_t g, const mpz_t a, const mpz_t b) {
    mpz_t a2, b2; // create and set new variables equal to the constants
    mpz_init_set(a2, a);
    mpz_init_set(b2, b);

    while (mpz_cmp_ui(b2, 0) != 0) {
        mpz_t t;
        mpz_init_set(t, b2);
        mpz_tdiv_r(b2, a2, b2); // b = a mod b
        mpz_set(a2, t);
        mpz_clear(t); // create and clear t with every loop
    }
    mpz_set(g, a2);
    mpz_clears(a2, b2, NULL);
}

void mod_inverse(mpz_t o, const mpz_t a, const mpz_t n) {
    mpz_t r, r1, o1;
    mpz_init_set(r, n);
    mpz_init_set(r1, a);
    mpz_set_ui(o, 0);
    mpz_init_set_ui(o1, 1);

    while (mpz_cmp_ui(r1, 0) != 0) {
        mpz_t q, temp, r_temp, o_temp; // r_temp and o_temp are auxiliary variables
        mpz_inits(q, temp, NULL);

        mpz_tdiv_q(q, r, r1); // q = r / r'

        mpz_init_set(r_temp, r);
        mpz_set(r, r1); // r = r'
        mpz_mul(temp, q, r1); // temp = q * r'
        mpz_sub(r1, r_temp, temp); // r' = r - temp

        mpz_init_set(o_temp, o);
        mpz_set(o, o1); // o = o'
        mpz_mul(temp, q, o1); // temp = q * o'
        mpz_sub(o1, o_temp, temp); // o' = o - temp

        mpz_clears(q, temp, r_temp, o_temp, NULL);
    }
    if (mpz_cmp_ui(r, 1) > 0) {
        mpz_set_ui(o, 0);
    }
    if (mpz_cmp_ui(o, 0) < 0) {
        mpz_add(o, o, n);
    }
    mpz_clears(r, r1, o1, NULL);
}

void pow_mod(mpz_t o, const mpz_t a, const mpz_t d, const mpz_t n) {
    //initialize the mpz_t variables to be used in this function
    mpz_t d2, p, temp;
    mpz_inits(d2, p, temp, NULL);

    mpz_set(d2, d); //set d2 to d, since d is a const and can't be changed.
    mpz_set(p, a);
    mpz_set_ui(o, 1);

    while (mpz_sgn(d2) > 0) {
        if (mpz_odd_p(d2)) {
            mpz_mul(temp, o, p); // temp = o * p
            mpz_tdiv_r(o, temp, n); // o = temp mod c
        }
        mpz_mul(temp, p, p); // temp = p * p
        mpz_tdiv_r(p, temp, n); // p = temp mod c
        mpz_tdiv_q_ui(d2, d2, 2);
    }
    //free memory of variables
    mpz_clear(d2);
    mpz_clear(p);
    mpz_clear(temp);
}

bool is_prime(const mpz_t n, uint64_t iters) {
    // corner cases if n = 0, 1, 2, or even
    if (mpz_cmp_ui(n, 2) < 0) {
        return 0;
    }
    if (mpz_cmp_ui(n, 2) == 0) {
        return 1;
    }
    if (mpz_even_p(n) != 0) {
        return 0;
    }

    // set our variables used to write n - 1 = 2 ^ s * r
    // set n-1 and s-1, used later
    mpz_t divisor, s, r, n_1, s_1, range;
    mpz_init_set_ui(divisor, 1);
    mpz_inits(n_1, r, s, range, NULL);
    mpz_sub_ui(range, n, 4); // n - 4
    mpz_sub_ui(n_1, n, 1); // div n  - 1

    while (mpz_even_p(r)) {
        mpz_div(r, n_1, divisor); // r = (n - 1) / d
        mpz_mul_ui(divisor, divisor, 2); // update divisor, 2^s = 2(s+1)
        mpz_add_ui(s, s, 1); // s = (s+1)
    }

    mpz_init(s_1); // s-1 can be calculated after s is found
    mpz_sub_ui(s_1, s, 1);

    mpz_t random_value, y;
    mpz_inits(random_value, y, NULL);

    for (uint64_t i = 1; i < iters; i++) {
        mpz_urandomm(random_value, state, range); // create a value in [0 - (n - 4)]
        mpz_add_ui(random_value, random_value, 2); // shift value to the right
        pow_mod(y, random_value, r, n);

        if ((mpz_cmp_ui(y, 1) != 0) && (mpz_cmp(y, n_1) != 0)) {
            mpz_t j, two, temp_y;
            mpz_init_set_ui(j, 1);
            mpz_init_set_ui(two, 2);
            mpz_init(temp_y);

            while ((mpz_cmp(j, s_1) <= 0) && (mpz_cmp(y, n_1) != 0)) {
                pow_mod(temp_y, y, two, n); // y = pow_mod(y, 2, n)
                mpz_set(y, temp_y);

                if (mpz_cmp_ui(y, 1) == 0) {
                    mpz_clears(j, two, y, temp_y, r, s, random_value, divisor, n_1, s_1, range,
                        NULL); // clear all values
                    return false;
                }
                mpz_add_ui(j, j, 1);
            }

            mpz_clears(j, two, temp_y, NULL);
            if (mpz_cmp(y, n_1) != 0) {

                mpz_clears(y, r, s, random_value, divisor, n_1, s_1, range,
                    NULL); // clear all values
                return false;
            }
        }
    }

    // clear variables at the end
    mpz_clears(y, r, s, random_value, divisor, n_1, s_1, range, NULL);
    return true;
}

void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    mpz_t rand_num;
    mpz_init(rand_num);

    // generate a random number of bits length
    mpz_urandomb(rand_num, state, bits);

    while (!is_prime(rand_num, iters)) {

        mpz_urandomb(rand_num, state, bits);
    }
    mpz_set(p, rand_num);
    mpz_clear(rand_num);
}
