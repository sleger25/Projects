#include <stdio.h>
#include <gmp.h>
#include <stdint.h>
#include <stdlib.h>
#include "randstate.h"

gmp_randstate_t state; //declaring the global variable, "state"

void randstate_init(uint64_t seed) {
    gmp_randinit_mt(state); //creates a gmp_randstate_t object
    gmp_randseed_ui(state, seed); //seeds the generator object, "state" using the seed
}

void randstate_clear(void) {
    gmp_randclear(state);
}
