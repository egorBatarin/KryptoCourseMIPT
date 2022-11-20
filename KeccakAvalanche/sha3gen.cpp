#include "sha3algo.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <unistd.h>

// init params as in FIPS202 srandart
void init_internals(internal_t *s) {
  s->LSFR_state = 0x01;
  s->LFSR = true;
  s->num_rounds = 24;
}

void parse_help(int argc, char* argv[], const char* message) { //print help info
    for (int i = 0; i < argc; ++i) {
        const char* help = argv[i];
        if (!strcmp("-h", help) || !strcmp("--help", help)) {
            printf("%s\n", message);
            exit(0);
        }
    }
}

int main(int argc, char** argv) {
    const char* message = \
    "Calculates sha-3 hash sum.\n"
    "\tArgs: <input_string> [<hash_size>]\n"
    "hash_size values: 224, 256, 384, 512\n";

    parse_help(argc, argv, message);

    if (argc == 1) {
        printf("%s", message);
        exit(-1);
    }

    int hash_size = 224; //initialize
    if (argc > 2) {
        hash_size = atoi(argv[2]);
        assert(hash_size == 224 || hash_size == 384 || hash_size == 256 || hash_size == 512);
    }

    const char* input = argv[1];
    const unsigned long long inputByteLen = strlen(input);
    dprintf(STDOUT_FILENO, "sha-%d input[%llu]\t%s\n", hash_size, inputByteLen, input); // print ssa-hash_size/ input size/ input string

    unsigned char* output = (unsigned char*)calloc(hash_size >> 3, sizeof(char)); // output hash memory
    assert(output != NULL);

    /* Calculate dev SHA3-XXX -  */
    dprintf(STDOUT_FILENO, "dev sha-%d\n", hash_size);
    internal_t params;
    init_internals(&params); // declare params for Keccak devfunc
    Keccak_Dev(1600 - (hash_size << 1), (hash_size << 1), (const unsigned char*)input, inputByteLen, 0x06, output, hash_size >> 3, &params); //write result to output
    for (int i = 0; i < hash_size >> 3; ++i) {
        dprintf(STDOUT_FILENO, "%02x", output[i]); //print output like 00 FA 1B ...
    }
    dprintf(STDOUT_FILENO, "\n");
    memset(output, 0x0, hash_size / 8); // clear output memory

     /* Calculate standart FIPS202 SHA3-XXX */
    dprintf(STDOUT_FILENO, "fips202 sha-%d\n", hash_size);
    Keccak(1600 - (hash_size << 1), (hash_size << 1), (const unsigned char*)input, inputByteLen, 0x06, output, hash_size >> 3);
    for (int i = 0; i < hash_size >> 3; ++i) {
        dprintf(STDOUT_FILENO, "%02x", output[i]);
    }
    dprintf(STDOUT_FILENO, "\n");
    memset(output, 0x0, hash_size / 8);


    /* Calculate FIPS202 SHA3-224 */
    dprintf(STDOUT_FILENO, "fips202 sha-224\n");
    FIPS202_SHA3_224((const unsigned char*)input, inputByteLen, output);

    for (int i = 0; i < 224 >> 3; ++i) {
        dprintf(STDOUT_FILENO, "%02x", output[i]);
    }
    dprintf(STDOUT_FILENO, "\n");

    free(output);
    return 0;
}
