#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "trie.h"
#include "code.h"

// received general instruction on tries and implementation by TAs/tutors

// create a node with pointers to children
TrieNode *trie_node_create(uint16_t index) {
    // allocate memory for node
    TrieNode *trie_node = (TrieNode *) (calloc(1, sizeof(TrieNode)));
    if (trie_node == NULL) {
        return NULL;
    }
    // each of the children node pointers are NULL
    for (uint16_t i = 0; i < ALPHABET; i++) {
        trie_node->children[i] = NULL;
    }
    // set code to index
    trie_node->code = index;
    return trie_node;
}

// delete a node
void trie_node_delete(TrieNode *n) {
    if (n != NULL) {
        free(n);
        n = NULL;
    }
}

// Initializes a trie: a root TrieNode with the code EMPTY_CODE.
TrieNode *trie_create(void) {
    return trie_node_create(EMPTY_CODE);
}

// delete children but not root
void trie_reset(TrieNode *root) {
    if (root == NULL) {
        return;
    }
    // delete trie children
    for (int i = 0; i < ALPHABET; i++) {
        trie_delete(root->children[i]);
        root->children[i] = NULL;
    }
}

// delete children nd root
void trie_delete(TrieNode *n) {
    if (n == NULL) {
        return;
    }
    // delete trie children
    for (int i = 0; i < ALPHABET; i++) {
        if (n->children[i] != NULL) {
            trie_delete(n->children[i]);
        }
    }
    // delete trie
    trie_node_delete(n);
}

// returns a pointer to the child node reprsenting the symbol sym.
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    return n->children[sym];
}
