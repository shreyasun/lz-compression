#include "trie.h"
#include "code.h"
#include "word.h"
#include "io.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include <unistd.h>
#include <sys/stat.h>
#include "inttypes.h"
#include <fcntl.h>
#define OPTIONS "vi:o:h"

void help(void);
uint16_t bit_length(uint16_t x);

void usage(char *exec) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "   Compresses files using the LZ78 compression algorithm.\n"
        "   Compressed files are decompressed with the corresponding decoder.\n"
        "\n"
        "USAGE\n"
        "   %s [-vh] [-i input] [-o output]\n"
        "\n"
        "OPTIONS\n"
        "   -v             Display compression statistics.\n"
        "   -i input       Specify input to compress (default: stdin).\n"
        "   -o output      Specify output of compressed input (default: stdout).\n"
        "   -h             Display program help and output.\n",
        exec);
}

// Recieved TA/tutor help for file setup
 
int main(int argc, char **argv) {
    bool print_stats = false;
    int infile = STDIN_FILENO; // stdin
    int outfile = STDOUT_FILENO; // stdout

    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'v': print_stats = true; break;
        // input file
        case 'i':
            infile = open(optarg, O_RDONLY);
            if (infile < 0) {
                printf("Failed to open %s.\n", optarg);
                return -1;
            }
            break;
        // output file
        case 'o':
            outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR);
            if (infile < 0) {
                printf("Failed to open %s.\n", optarg);
                return -1;
            }
            break;
        // help
        case 'h':
            help();
            return 0;
            break;
        default:
            usage(argv[0]); 
            return EXIT_FAILURE;
        }
    }

    struct stat buf;
    fstat(infile, &buf);
    // The file size and the protection bit mask you will obtain using fstat()
    fchmod(outfile, buf.st_mode);
    // Write the filled out file header to outfile using write_header(). This means writing out the struct itself to the file
    FileHeader *header_out = calloc(1, sizeof(FileHeader));
    header_out->magic = MAGIC;
    header_out->protection = buf.st_mode;
    write_header(outfile, header_out);
    free(header_out);

    // Create a trie
    TrieNode *root = trie_create();
    // root node copy
    TrieNode *curr_node = root;
    // previous trie node
    TrieNode *prev_node = NULL;
    // current symbol
    uint8_t curr_sym = 0;
    // previously read symbol
    uint8_t prev_sym = 0;
    // monotonic counter
    uint16_t next_code = START_CODE;

    // Use read_sym() in a loop to read in all the symbols from infile - each symbol should be curr_sym
    while (read_sym(infile, &curr_sym) == true) {
        // Set next_node
        TrieNode *next_node = trie_step(curr_node, curr_sym);
        // If next_node is not NULL, that means we have seen the current prefix. Set prev_node to be curr_node and then curr_node to be next_node.
        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            // write the pair (curr_node->code, curr_sym) add current prefix to tree
            write_pair(outfile, curr_node->code, curr_sym, bit_length(next_code));
            // new trie node whose code is next_code
            curr_node->children[curr_sym] = trie_node_create(next_code);
            // Reset curr_node to point at the root of the trie
            curr_node = root;
            // increment the value of next_code
            next_code = next_code + 1;
        }
        // Check if next_code is equal to MAX_CODE. If it is, use trie_reset() to reset the trie to just having the root node.
        if (next_code == MAX_CODE) {
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }
        // Update prev_sym to be curr_sym.
        prev_sym = curr_sym;
    }
    // check if curr_node points to the root trienode
    if (curr_node != root) {
        // Write the pair
        write_pair(outfile, prev_node->code, prev_sym, bit_length(next_code));
        // increment next_code
        next_code = (next_code + 1) % MAX_CODE;
    }
    // Write the pair (STOP_CODE, 0)
    write_pair(outfile, STOP_CODE, 0, bit_length(next_code));
    // flush the contents of buffer
    flush_pairs(outfile);

    if (total_bits % 8 != 0) {
        total_bits += 8;
    }

    if (print_stats) {
        int uncompressed_size = total_syms - 1;
        int compressed_size = (total_bits / 8);
        double space_saved = 100 * (1 - ((double) compressed_size / uncompressed_size));
        printf("Compressed file size: %d bytes\n", compressed_size);
        printf("Uncompressed file size: %d bytes \n", uncompressed_size);
        printf("Compression ratio: %.2f%%\n", space_saved);
    }

    // close files
    close(infile);
    close(outfile);

    // delete trie
    trie_delete(root);

    return 0;
}

// calculate bit length
uint16_t bit_length(uint16_t x) {
    uint16_t n = 0;
    while (x != 0) {
        x = (x >> 1);
        n += 1;
    }
    return n;
}

void help(void) {
    printf("SYNOPSIS\n"
           "   Compresses files using the LZ78 compression algorithm.\n"
           "   Compressed files are decompressed with the corresponding decoder.\n"
           "\n"
           "USAGE\n"
           "   ./encode [-vh] [-i input] [-o output]\n"
           "\n"
           "OPTIONS\n"
           "   -v             Display compression statistics.\n"
           "   -i input       Specify input to compress (default: stdin).\n"
           "   -o output      Specify output of compressed input (default: stdout).\n"
           "   -h             Display program help and output.\n");
}
