#pragma once
#include <stdint.h>

#define SHA3_FUNC(hash_size, input, inputByteLen, output) FIPS202_SHA3_##hash_size(input, inputByteLen, output)

void print_hash(const unsigned hashBitlen, const unsigned char* input, const unsigned inputBytelen);

void dump_hash(const unsigned bits_len, const unsigned char *hash);

/**
  *  Function to compute SHA3-224 on the input message. The output length is fixed to 28 bytes.
  */
void FIPS202_SHA3_224(const unsigned char *input, unsigned int inputByteLen, unsigned char *output);

/**
  *  Function to compute SHA3-256 on the input message. The output length is fixed to 32 bytes.
  */
void FIPS202_SHA3_256(const unsigned char *input, unsigned int inputByteLen, unsigned char *output);

/**
  *  Function to compute SHA3-384 on the input message. The output length is fixed to 48 bytes.
  */
void FIPS202_SHA3_384(const unsigned char *input, unsigned int inputByteLen, unsigned char *output);

/**
  *  Function to compute SHA3-512 on the input message. The output length is fixed to 64 bytes.
  */
void FIPS202_SHA3_512(const unsigned char *input, unsigned int inputByteLen, unsigned char *output);

void Keccak(unsigned int rate, unsigned int capacity, const unsigned char *input, unsigned long long int inputByteLen, unsigned char delimitedSuffix, unsigned char *output, unsigned long long int outputByteLen);
/* Developer version */

/**
  * SHA3_224: r=1600-c, c=224*2, suffix=0x06, outpuByteLen=224/8
  * SHA3_256: r=1600-c, c=256*2, suffix=0x06, outpuByteLen=256/8
  * SHA3_384: r=1600-c, c=384*2, suffix=0x06, outpuByteLen=384/8
  * SHA3_512: r=1600-c, c=512*2, suffix=0x06, outpuByteLen=512/8
  */ 

typedef struct internal_t {
  bool LFSR; // allow random permutation
  uint8_t LSFR_state; // init rand generator
  unsigned num_rounds; // num of rounds
} internal_t;

// set default params for FIPS_202 functions
void init_internals_fips202(internal_t *s);

// dev functions with debug prints
void Keccak_Dev(unsigned int rate, unsigned int capacity, const unsigned char *input, unsigned long long int inputByteLen, unsigned char delimitedSuffix, unsigned char *output, unsigned long long int outputByteLen, const internal_t* params);
void KeccakF1600_StatePermute_Dev(void *state, const internal_t *params);