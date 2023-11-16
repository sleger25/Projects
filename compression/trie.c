#include <stdint.h>
#include <stdlib.h>

//Header Files
#include "code.h"
#include "trie.h"

/*
 * Creates a new TrieNode and returns a pointer to it
 * Allocate memory for TrieNode
 * Code is the code to be assigned to this new node
 * Returns the newly allocated node
 */
TrieNode *trie_node_create(uint16_t code) {
    TrieNode *node = (TrieNode *) malloc(sizeof(TrieNode));
    node->code = code;
    for (int i = 0; i < ALPHABET; i++) {
        node->children[i] = NULL;
    }
    return node;
}

/*
 * Deletes Node n
 * Frees any allocated memory
 */
void trie_node_delete(TrieNode *n) {
    if (n != NULL) {
        for (int i = 0; i < ALPHABET; i++) {
            trie_node_delete(n->children[i]);
        }
        free(n);
    }
}

/*
 * Constructor: Creates the root TrieNode and returns a pointer to it
 * Allocate memory for TrieNode
 * Code is START_CODE
 * Returns the newly allocated node
 */
TrieNode *trie_create(void) {
    TrieNode *root = trie_node_create(EMPTY_CODE);
    if (root != NULL) {
        return root;
    } else {
        return NULL;
    }
}

/*
 * Resets the trie: called when code reaches MAX_CODE
 * Deletes all the children of root and frees allocated memory
 */
void trie_reset(TrieNode *root) {
    for (int i = 0; i < ALPHABET; i++) {
        trie_node_delete(root->children[i]);
        root->children[i] = NULL;
    }
}

/*
 * Destructor: Deletes all nodes starting at n as the root
 * Deletes any children recursively before deleting the root
 * Frees all the memory allocated for TrieNodes n and below
 */
void trie_delete(TrieNode *n) {
    if (n != NULL) {
        for (int i = 0; i < ALPHABET; i++) {
            trie_delete(n->children[i]);
            n->children[i] = NULL;
        }
        free(n);
    }
}

/*
 * Checks if node has any children called sym
 * Returns the address if found, NULL if absent
 */
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    if (n != NULL) {
        return n->children[sym];
    } else {
        return NULL;
    }
}
