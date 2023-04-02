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

uint16_t bit_length(uint16_t x);
void help(void);

void usage(char *exec) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "   Decompresses files using the LZ78 decompression algorithm.\n"
        "   Used with files compressed with the corresponding encoder.\n"
        "\n"
        "USAGE\n"
        "   %s [-vh] [-i input] [-o output]\n"
        "\n"
        "OPTIONS\n"
        "   -v             Display decompression statistics.\n"
        "   -i input       Specify input to compress (stdin by default).\n"
        "   -o output      Specify output of decompressed input (stdout by default).\n"
        "   -h             Display program usage\n",
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
                printf("%s: No such file or directory\n", optarg);
                return -1;
            };
            break;
        // output file
        case 'o':
            outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR);
            if (outfile < 0) {
                printf("%s: No such file or directory\n", optarg);
                return -1;
            };
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

    // Read in the file header with read_header()
    FileHeader *header_in = calloc(1, sizeof(FileHeader));
    read_header(infile, header_in);
    fchmod(outfile, header_in->protection);

    free(header_in);

    // Create a new word table with wt_create()
    WordTable *table = wt_create();
    uint8_t curr_sym = 0;
    uint16_t curr_code = 0;
    // next_code should be initialized as START_CODE and functions exactly the same as the monotonic counter
    uint16_t next_code = START_CODE;
    // Use read_pair() in a loop to read all the pairs from infile.
    while (read_pair(infile, &curr_code, &curr_sym, bit_length(next_code))) {
        // append the read symbol with the word de- noted by the read code and add the result to table at the index next_code
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        // Write the word
        write_word(outfile, table[next_code]);
        // increment next_code
        next_code = next_code + 1;
        // if next_code equals MAX CODE, reset the table and set next_code to be START_CODE
        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }
    // flush buffered words
    flush_words(outfile);
    
    // verbose output
    if (total_bits % 8 != 0) {
        total_bits += 8;
    }

    if (print_stats) {
        int uncompressed_size = total_syms;
        int compressed_size = (total_bits / 8);
        double space_saved = 100 * (1 - ((double) compressed_size / uncompressed_size));
        printf("Compressed file size: %d bytes\n", compressed_size);
        printf("Uncompressed file size: %d bytes \n", uncompressed_size);
        printf("Compression ratio: %.2f%%\n", space_saved);
    }

    // close files
    close(infile);
    close(outfile);

    // delete table
    wt_delete(table);

    return 0;
}

// bit length
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
           "   Decompresses files using the LZ78 decompression algorithm.\n"
           "   Used with files compressed with the corresponding encoder.\n"
           "\n"
           "USAGE\n"
           "   ./decode [-vh] [-i input] [-o output]\n"
           "\n"
           "OPTIONS\n"
           "   -v             Display decompression statistics.\n"
           "   -i input       Specify input to compress (stdin by default).\n"
           "   -o output      Specify output of decompressed input (stdout by default).\n"
           "   -h             Display program usage\n");
}
