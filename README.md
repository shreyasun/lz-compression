## Lempel-Ziv Compression 
Lempel-Ziv Compression is a method of lossless compression that utilizes code-symbol pairs, tries, and a word table to perform file compression and decompression. This project implements encoding and decoding algorithms that utilize these data structures to perform the Lempel-Ziv Compression methods.

## If you are a current CSE 13S student, please do not look at my source code!

## Build
This project creates the executables 'encode' and 'decode'. Typing 'make' or 'make all' will build all of these executables and link all of the object files ('trie.o', 'word.o', and 'io.o'). Typing in 'make encode' or 'make decode' will build those executable binary files and their linked object files individually.
 
## Cleaning
Type 'make clean' to remove the executable binary files 'encode' and 'decode' and all of the .o files.

## Run Options
### Encode
Running './encode' followed by various command line options will compress a user specified file. Typing in './encode -h' will display command line options for encode. Typing './encode -i' followed by a file name will specify that file (if it exists) to be compressed. Otherwise, the program will accept standard input for compression. Typing './encode -o' followed by file name will place output of compressed input to that specified file. Otherwise, the program will yield compressed output to standard output. Typing './encode -v' will print compression statistics to stderr.  

### Decode
Running './decode' followed by various command line options will decompress a compressed file (encoded by running ./encode). Typing in './decode -h' will display command line options for decode. Typing './decode -i' followed by an encoded file will specify that file (if it exists) to be decompressed. Otherwise, the program will accept standard input for decompression. Typing './decode -o' followed by file name will place output of decompressed input to that specified file. Otherwise, the program will yield decompressed output to standard output. Typing './decode -v' will print compression statistics to stderr.         


