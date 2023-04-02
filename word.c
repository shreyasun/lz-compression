#include "word.h"
#include <stdlib.h>
#include <stdio.h>
#include "code.h"
#include <stdint.h>

// received general instruction on words and word tables, and implementation by TAs/tutors

Word *word_create(uint8_t *syms, uint32_t len) {
    // allocate memory for word
    Word *word = (Word *) calloc(1, sizeof(Word));
    if (word) {
        // length of the array of symbols is given by len
        word->len = len;
        if (syms == NULL) {
            word->syms = syms;
        } else {
            // create array of symbols for word based on length
            word->syms = calloc(len, sizeof(uint8_t));
            for (uint8_t i = 0; i < len; i++) {
                word->syms[i] = syms[i];
            }
        }
    }
    return word;
}

// add letter to word
Word *word_append_sym(Word *w, uint8_t sym) {
    // allocate memory for new word
    Word *new_word = (Word *) calloc(1, sizeof(Word)); //word_create(w->syms, w->len + 1);
    // new word is 1 letter longer (due to appended symbol)
    new_word->len = w->len + 1;
    // allocate new word's symbols space to 1 symbol longer than original word (due to appended symbol)
    new_word->syms = calloc(1, w->len + 1);
    // copy over symbols from word to new word
    for (uint8_t i = 0; i < w->len; i++) {
        new_word->syms[i] = w->syms[i];
    }
    // append new symbol to end of new word
    new_word->syms[w->len] = sym;
    return new_word;
}

// delete word
void word_delete(Word *w) {
    if (w == NULL) {
        return;
    }
    free(w->syms);
    free(w);
    w = NULL;
}

// create word table
WordTable *wt_create(void) {
    // allocate memory for word table to pre-defined size of MAX_CODE
    WordTable *word_table = (WordTable *) calloc(MAX_CODE, sizeof(WordTable));
    if (word_table == NULL) {
        return NULL;
    }
    // initialize with a single Word at index EMPTY_CODE
    word_table[EMPTY_CODE] = word_create(NULL, 0);
    return word_table;
}

// delete words but keep table
void wt_reset(WordTable *wt) {
    if (wt == NULL) {
        return;
    }
    // delete each word in table
    for (int i = 2; i < MAX_CODE; i++) {
        word_delete(wt[i]);
        wt[i] = NULL;
    }
}

// delete words and table
void wt_delete(WordTable *wt) {
    if (wt == NULL) {
        return;
    }
    // delete each word in table
    for (int i = 0; i < MAX_CODE; i++) {
        word_delete(wt[i]);
        wt[i] = NULL;
    }
    // delete word table
    free(wt);
    wt = NULL;
}
