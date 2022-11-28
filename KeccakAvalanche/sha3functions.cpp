#include "sha3algo.hpp"
#include <cstdio>
#include <cstdlib>
#include <cassert>

void dump_hash(const unsigned bits_len, const unsigned char *hash) {
    assert(hash != nullptr);
    
    const unsigned size = bits_len >> 3;
    for (unsigned i = 0; i < size; ++i) {
        printf("%02x ", hash[i]);
    }
    putchar('\n');
}

void print_hash(const unsigned hashBitlen, const unsigned char* input, const unsigned inputBytelen) {
    
    if (!(hashBitlen == 224 || hashBitlen == 256 || hashBitlen == 384 || hashBitlen == 512)) {
        printf("hash error\n");
        return;
    }

    printf("input len: %u(%u)\nhash len: %u(%u)\n", inputBytelen, inputBytelen * 8, hashBitlen >> 3, hashBitlen);
    
    unsigned char* hash = (unsigned char*)calloc(hashBitlen >> 3, sizeof(unsigned char));
    assert(hash != nullptr);
    
    switch(hashBitlen) {
        case 224:
            SHA3_FUNC(224, input, inputBytelen, hash);
            break;
        case 256:
            SHA3_FUNC(256, input, inputBytelen, hash);
            break;
        case 384:
            SHA3_FUNC(384, input, inputBytelen, hash);
            break;
        case 512:
            SHA3_FUNC(512, input, inputBytelen, hash);
            break;
        default:
            printf("hash error\n");
    }
    
    
    dump_hash(hashBitlen, hash);

}