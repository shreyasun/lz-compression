#include "word.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "io.h"
#include "endian.h"
#include <unistd.h>
#include "code.h"
#include <sys/stat.h>
#include <assert.h>

// received general instruction on and how to implement i/o functions and bit vectors from CSE 13S TA/tutors 

// read_pair, write_pair, flush_pairs
static uint8_t bit_buffer[BLOCK];
static int bit_index = 0;
static int bytesread = 0;
// read_sym, write_words, flush_words
static uint8_t sym_buffer[BLOCK]; // buffer of size block - 4 K
static int cur_buf_index = 0;

// extern variables for file size count
uint64_t total_syms; // To count the symbols processed.
uint64_t total_bits; // To count the bits processed.

// read bytes from file
int read_bytes(int infile, uint8_t *buf, int to_read) {
    if (to_read == 0) {
        return 0;
    }
    // current bytes read (for each iteration)
    int bytes_read = 0;
    // total bytes read so far
    int bytes_count = 0;
    // keep reading bytes until there's no more
    while (to_read != 0) {
        // read bytes
        bytes_read = read(infile, buf + bytes_count, to_read);
        // stop reading if EOF or error
        if (bytes_read == -1 || bytes_read == 0) {
            return bytes_count;
        }
        // increment total bytes with how many has been read in this round
        bytes_count += bytes_read;
        // decrement how much is left to read
        to_read -= bytes_read;
    }
    return bytes_count;
}

// write bytes to file
int write_bytes(int outfile, uint8_t *buf, int to_write) {
    if (to_write == 0) {
        return 0;
    }
    // current bytes written so far
    int bytes_written = 0;
    // total bytes written so far
    int bytes_count = 0;
    // keep writing bytes until there's no more
    while (to_write != 0) {
        // write bytes
        bytes_written = write(outfile, buf + bytes_count, to_write);
        // stop writing if error
        if (bytes_written == -1) {
            return bytes_count;
        }
        // increment total bytes with how many has been written in this round
        bytes_count += bytes_written;
        // decrement how much is left to write
        to_write -= bytes_written;
    }
    return bytes_count;
}

// read magic/protection from header of file
void read_header(int infile, FileHeader *header) {
    // read bytes into outfile 
    int readbytes = read_bytes(infile, (uint8_t *) (header), sizeof(FileHeader));
    total_bits += (readbytes * 8);
    // swap header magic and protection values if big endian
    if (!(little_endian())) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    // verify the magic number ensure magic number is correct 
    assert(header->magic == MAGIC);
}

// write magic/protection to header of file
void write_header(int outfile, FileHeader *header) {
    // swap header magic and protection values if big endian - help from tutor Ben
    if (!(little_endian())) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    // write bytes into outfile - from tutor Ben
    int writebytes = write_bytes(outfile, (uint8_t *) (header), sizeof(FileHeader));
    total_bits += (writebytes * 8);
}

// read one symbol
bool read_sym(int infile, uint8_t *sym) {
    // Initialize read bytes count
    static int length = 0;
    // If current index is 0, then calculate length by reading bytes in file.
    if (cur_buf_index == 0) {
        length = read_bytes(infile, sym_buffer, BLOCK);
    }
    // get the current symbol
    *sym = sym_buffer[cur_buf_index];
    // Increment current buffer index
    cur_buf_index++;
    // reset index to 0 if number of bytes read is equal to current index
    if (length == cur_buf_index) {
        cur_buf_index = 0;
    }
    // increment total syms
    total_syms++;
    // return if index is less than length + 1
    return (cur_buf_index < length + 1);
}

// write code, symbol pair
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    int buf_end = BLOCK * 8; // bit size of buffer
    // bits for code
    // iterate through bitlen amount of bits
    for (uint8_t i = 0; i < bitlen; i++) {
        // increment total bits
        total_bits++;
        // get bit at position i
        uint8_t bit = (code >> (i % 16)) & (1);
        if (bit == 1) {
            // set bit
            bit_buffer[bit_index / 8] |= (0x1 << bit_index % 8);
        } else {
            // clear bit
            bit_buffer[bit_index / 8] &= ~(0x1 << bit_index % 8);
        }
        // increment bit index
        bit_index++;
        // write bytes if buffer is full and set index to 0
        if (bit_index == buf_end) {
            write_bytes(outfile, bit_buffer, BLOCK);
            bit_index = 0;
        }
    }
    // bits for sym
    // iterate through 8 bits
    for (uint8_t i = 0; i < 8; i++) {
        // increment total bits
        total_bits++;
        // get bit at position i
        uint8_t bit = (sym >> (i % 8)) & (1);
        if (bit == 1) {
            // set bit
            bit_buffer[bit_index / 8] |= (0x1 << bit_index % 8);
        } else {
            // clear bit
            bit_buffer[bit_index / 8] &= ~(0x1 << bit_index % 8);
        }
        // increment bit index
        bit_index++;
        // write bytes if buffer is full and set index to 0
        if (bit_index == buf_end) {
            write_bytes(outfile, bit_buffer, BLOCK);
            bit_index = 0;
        }
    }
}

// write any remaining pairs 
void flush_pairs(int outfile) {
    int bits_num;
    // determine bit index to write out to
    if (bit_index % 8 == 0) {
        bits_num = bit_index / 8;
    } else {
        bits_num = (bit_index / 8) + 1;
    }
    // write bytes to outfile
    write_bytes(outfile, bit_buffer, bits_num);
    // reset index
    bit_index = 0;
}

// read code, symbol pair
bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    // reset code
    *code = 0;
    // iterate through bitlen amount of bits
    for (int i = 0; i < bitlen; i++) {
        // increment total bits
        total_bits++;
        // check when to read bytes and reset index to 0 
        if (bit_index == bytesread * 8) {
            // get number of bytes read
            bytesread = read_bytes(infile, bit_buffer, BLOCK);
            // reset index
            bit_index = 0;
        }
        // get bit 
        uint8_t bit = (bit_buffer[bit_index / 8] >> (bit_index % 8)) & (1);
        // The “read” code is placed in the pointer to code
        if (bit == 1) {
            *code = *code | (0x1 << i);
        } else {
            *code = *code & ~(0x1 << i);
        }
        // increment index
        bit_index++;
    }
    // sym
    // iterate through 8 bits
    for (uint8_t i = 0; i < 8; i++) {
        // increment total bits
        total_bits++;
        // check when to read bytes and reset index to 0 
        if (bit_index == bytesread * 8) {
            // get number of bytes read
            bytesread = read_bytes(infile, bit_buffer, BLOCK);
            // reset index
            bit_index = 0;
        }
        // get bit
        uint8_t bit = (bit_buffer[bit_index / 8] >> (bit_index % 8)) & (1);
        // The “read” symbol is placed in the pointer to sym
        if (bit == 1) {
            *sym = *sym | (0x1 << i);
        } else {
            *sym = *sym & ~(0x1 << i);
        }
        // increment index
        bit_index++;
    }
    // Returns true if there are pairs left to read in the buffer, else false (based on if read code is not STOP_CODE)
    return !(*code == STOP_CODE);
}

// write word to file
void write_word(int outfile, Word *w) {
    if (w == NULL) {
        return;
    }
    // iterate through word
    for (uint8_t i = 0; i < w->len; i++) {
        // increment total syms
        total_syms++;
        // if index is BLOCK, write bytes to outfile and reset index
        if (cur_buf_index == BLOCK) {
            write_bytes(outfile, sym_buffer, BLOCK);
            cur_buf_index = 0;
        }
        // put symbol to buffer if buffer is not yet full
        sym_buffer[cur_buf_index] = w->syms[i];
        // increment
        cur_buf_index++;
    }
}

// write any remaining words to file
void flush_words(int outfile) {
    // write out remaining bytes to outfile
    write_bytes(outfile, sym_buffer, cur_buf_index);
    cur_buf_index = 0;
}
