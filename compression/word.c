#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Header Files
#include "code.h"
#include "word.h"

/*
 * Creates a new Word with symbols syms and length len
 * Allocates new array and copies the symbols over
 */
Word *word_create(uint8_t *syms, uint32_t len) {
    // Allocate memory for word object and array of syms
    Word *w = (Word *) malloc(sizeof(Word));
    uint8_t *new_syms = (uint8_t *) malloc(len * sizeof(uint8_t));

    // Loop through the the syms array and copy it over
    for (uint32_t i = 0; i < len; i++) {
        new_syms[i] = syms[i];
    }

    w->syms = new_syms;
    w->len = len;

    return w;
}

/*
 * Creates a new word by appending symbol sym to word w
 * Updates the length of the new word and copies symbols over
 * Returns a pointer to the newly allocated word
 */
Word *word_append_sym(Word *w, uint8_t sym) {
    uint32_t new_len = w->len + 1;
    uint8_t *new_syms = (uint8_t *) malloc(new_len * sizeof(uint8_t));
    for (uint32_t i = 0; i < w->len; i++) {
        new_syms[i] = w->syms[i];
    }

    new_syms[new_len - 1] = sym; // Append the new symbol to the end of the array

    Word *new_w = word_create(new_syms, new_len); // Create new word object
    if (new_w == NULL) {
        free(new_syms);
        return NULL;
    }

    return new_w;
}

/*
 * Deletes the word w
 * Frees up associated space
 */
void word_delete(Word *w) {
    free(w->syms);
    free(w);
}

/*
 * Constructor:
 * Creates a new table big enough to fit MAX_CODE
 * Creates the first element at EMPTY_CODE and returns it
 */
WordTable *wt_create(void) {
    // Dynamically allocate memory for the WordTable
    WordTable *wt = (WordTable *) malloc(sizeof(Word *) * MAX_CODE);

    // Create the first (empty) word in the table
    wt[EMPTY_CODE] = word_create(NULL, 0);

    return wt;
}

/*
 * Deletes all words except EMPTY_CODE
 * Frees associated memory
 */
void wt_reset(WordTable *wt) {
    // Iterate through word table
    for (int i = EMPTY_CODE + 1; i < MAX_CODE; i++) {
        if (wt[i] != NULL) {
            wt[i] = NULL; // If word is not NULL, make it NULL
        }
    }
}

/*
 * Destructor: Deletes all words and tables
 * Frees up associated memory
 */
void wt_delete(WordTable *wt) {
    // Iterate through word table
    for (int i = 0; i < MAX_CODE; i++) {
        if (wt[i] != NULL) {
            word_delete(wt[i]); // If word is not NULL, delete it
        }
    }
    free(wt);
}
