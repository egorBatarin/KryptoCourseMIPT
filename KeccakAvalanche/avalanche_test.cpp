#include "sha3algo.hpp"
#include <cstdio>
#include <cassert>
#include <cstring>
#include "fileio.hpp"
#include <unistd.h>
#include <cstdint>


//#define HASH_SIZE 224

#define DELIMITED_SUFFIX 0x06
#define KECCAK_f1600 1600

size_t g_bit_changes_total[512] = {0};
size_t g_changes_total = 0;
size_t g_iterations_total = 0;

size_t g_bit_changes_local[512] = {0};
size_t g_changes_local = 0;
size_t g_iterations_local = 0;

void collect_bit_stat(const int hash_size, const unsigned char* hash, const unsigned char* hash0) {
    
    for (int byte_idx = 0; byte_idx < hash_size / 8; ++byte_idx) {
        // calculate diff
        unsigned char byte = hash0[byte_idx] ^ hash[byte_idx];
        
        for (int bit_idx = 0; bit_idx < 8; ++bit_idx) {
            unsigned char bit = (byte >> bit_idx) & 0x1;
           
            // register diff           
            if (bit == 1) {
                g_bit_changes_local[byte_idx * 8 + bit_idx]++;
                g_changes_local++;
            }
        }
    }
    g_iterations_local++;
    g_iterations_total++;
}

int flush_bit_stat(int fd, size_t inLenBits, const int hash_size, bool bit_stat) {
    if (g_iterations_local == 0)
        return -1;

    if (fd >= 0) {
        // idx | inputLen | changes per iter (av %)
        assert(dprintf(fd, "\n%3lu\t%8lu\t%8lu\t", g_iterations_total, inLenBits, g_changes_local) > 0);
    }
    
    for (int bit_idx = 0; bit_idx < hash_size; ++ bit_idx) {
        
        if (fd >= 0 && bit_stat) {
            // bit changes (av %) | ... |
           assert(dprintf(fd, "%6lu ", g_bit_changes_local[bit_idx]) > 0); 
        }
        

        g_bit_changes_total[bit_idx] += g_bit_changes_local[bit_idx];
        g_bit_changes_local[bit_idx] = 0;
    }
    
    g_changes_total += g_changes_local;
    g_changes_local = 0;

    g_iterations_local = 0;
    // g_iterations_global is incremented in collect_bit_stat

    return 0;
}

int dump_global_stat(int fd, const int hash_size, bool bit_stat, const int num_rounds) {
    assert(fd >= 0);
    dprintf(fd, "\ntotal(#)\taverage(%%)\tbit_stat(%%)\n");
    dprintf(fd, "%8lu\t%8.3f\t", g_iterations_total, 100. * g_changes_total / (g_iterations_total * hash_size));
    
    for (int bit_idx = 0; bit_stat && bit_idx < hash_size; ++ bit_idx) {
        // bit changes (av %) | ... |
        dprintf(fd, "%8.3f ", 100. * g_bit_changes_total[bit_idx] / g_iterations_total); 
    }
    
    dprintf(fd, "\n");
    dprintf(fd, "hash_len(bits)\trounds\n%8u\t%6u\n", hash_size, num_rounds);
}

int save_hash_result(int fd, const int hash_size, const unsigned char* hash) {
    assert(hash != nullptr);
    for (off_t i = 0; i < hash_size / 8; ++i) {
        assert(dprintf(fd, "%02x", hash[i]) > 0);
    }
    dprintf(fd, "\n");
}

ssize_t len_without_newline(const char* buf, const size_t max_len) {
    char* newline = (char*)memchr(buf, '\n', max_len);
    if (newline == nullptr)
        return -1;
    
    return newline - buf;
}

int main(int argc, char** argv) {
    const char* message = \
        "Avalanche test\n" \
        "\tArgs: <hash_size> <num_rounds> <LSFR_state>\n" \
        "\tif LSFR_state < 0 then exclude l-permutation\n";

    printf("%s\n", message);

    if (argc != 4) {
        perror("wrong args");
        return -1;
    }
    const unsigned hash_size = atoi(argv[1]);
    assert(hash_size == 224 || hash_size == 256 || hash_size == 384 || hash_size == 512);
    const uint8_t num_rounds = atoi(argv[2]);
    assert(num_rounds <= 24);
    const long LSFR_state = atol(argv[3]);
    bool LSFR = true;
    if (LSFR_state < 0) {
        LSFR = false;
    }

    internal_t params;
    params.LFSR = LSFR;
    params.LSFR_state = LSFR_state;
    params.num_rounds = num_rounds;

    dprintf(STDOUT_FILENO, "hash size: %u\trounds: %u\tLSFR[%u] %lu\n", hash_size, num_rounds, LSFR, LSFR_state);

    unsigned char* hash0 = (unsigned char*)calloc(hash_size / 8, sizeof(unsigned char));
    unsigned char* hash = (unsigned char*)calloc(hash_size / 8, sizeof(unsigned char));
    
    // hash log file
    int hash_fd = open("output_hash.log", O_WRONLY | O_TRUNC | O_CREAT, 0644);
    // avalanche effect stat file
    int stat_fd = open("avalanche_stat.log", O_WRONLY | O_TRUNC | O_CREAT, 0644);
    dprintf(stat_fd, "%3s\t%6s\t%6s\t%16s", "#", "InLen(bits)", "~bits", "~bits");

    // read input as strings ended with newline
    fileio::ReadOnly f("avalanche_effect.txt", 0);
    char* ptr = (char*)f.getSegment();
    const size_t seg_size = f.getSegSize();
    unsigned int seg_num = 0;

    // input string
    char* start = ptr;
    ssize_t len0 = 0;
    // rest of input string (from prev seg)
    char* tmp = nullptr;
    size_t rest_len = 0;

    while(ptr != nullptr) {
        // loaded bytes from cur segment
        size_t loaded = 0;
        // compute 1st string (or tail of string from prev segment)
        ssize_t len = len_without_newline((char*)ptr, seg_size);

        if (len == -1) {
            printf("error: string without newline\n");
            exit(-1);
        }

        char* input = nullptr;
        if (rest_len != 0) {
            char* input = (char*)calloc(len + rest_len + 1, sizeof(char));
            assert(input != nullptr);
            assert(tmp != nullptr);
            assert(memcpy(input, tmp, rest_len) == input);
            assert(memcpy(input + rest_len, (char*)ptr, len + 1) == input + rest_len);
            free(tmp);
            tmp = nullptr;
            start = input;
        } else {
            start = ptr;
        }

        loaded += (len + 1);
        if (seg_num == 0) {
            // get hash
            //SHA3_FUNC(224, (const unsigned char*)start, (int)(len + rest_len), hash0);
            Keccak_Dev(KECCAK_f1600 - hash_size * 2, hash_size * 2, (const unsigned char*)start, (int)(len + rest_len), DELIMITED_SUFFIX, hash0, hash_size / 8, &params);
            // write(hash_fd, (unsigned char*)start, len + rest_len + 1);
            save_hash_result(hash_fd, hash_size, hash0);
        } else {
            assert(start != nullptr);
            // get hash
            //SHA3_FUNC(224, (const unsigned char*)start, (int)(len + rest_len), hash);
            Keccak_Dev(KECCAK_f1600 - hash_size * 2, hash_size * 2, (const unsigned char*)start, (int)(len + rest_len), DELIMITED_SUFFIX, hash, hash_size / 8, &params);
            // write(hash_fd, (unsigned char*)start, len + rest_len + 1);
            save_hash_result(hash_fd, hash_size, hash);
            
            // calculate avalanche stat
            if (len + rest_len == len0) {
                collect_bit_stat(hash_size, hash, hash0);
                flush_bit_stat(stat_fd, len0, hash_size, false);
            }
            
            // dump avalanche stat

            // save prev result
            memcpy(hash0, hash, hash_size / 8);
        }
        
        // clear rest for new segment processing
        len0 = len + rest_len;
        rest_len = 0;
        start = ptr;
        if (input != nullptr) {
            free(input);
        }  
        
        while (loaded < seg_size) {
            start += (len + 1); 
            len = len_without_newline(start, seg_size - loaded); 
            
            if (len == -1) {
                rest_len = seg_size - loaded;
                loaded = seg_size;
                continue;
            }
            
            loaded += (len + 1);

            //SHA3_FUNC(224, (const unsigned char*)start, (int)(len), hash);
            Keccak_Dev(KECCAK_f1600 - hash_size * 2, hash_size * 2, (const unsigned char*)start, (int)(len), DELIMITED_SUFFIX, hash, hash_size / 8, &params);
            // write(hash_fd, (unsigned char*)start, len + 1);
            save_hash_result(hash_fd, hash_size, hash);
            // do analysis
            if (len == len0) {
                collect_bit_stat(hash_size, hash, hash0);
                flush_bit_stat(stat_fd, len0, hash_size, false);
            }

            // save prev result
            memcpy(hash, hash0, hash_size / 8);
            len0 = len;
        }

        // save rest of prev segment
        if (rest_len > 0) {
            tmp = (char*)calloc(rest_len, sizeof(char));
            assert(tmp != nullptr);
            assert(memcpy(tmp, start, rest_len) == tmp);
        }

        // load next file segment
        ptr = (char*)f.nextSegment();
        seg_num++;
        // printf("loaded seg: %u\n", seg_num);
    }

    dump_global_stat(stat_fd, hash_size, true, num_rounds);
    dump_global_stat(STDOUT_FILENO, hash_size, false, num_rounds);
/*
    printf("\n\n\n");
    print_hash(224, (const unsigned char*)"sasha", 0);
    print_hash(224, (const unsigned char*)"savsha", 0);
    print_hash(224, (const unsigned char*)"sabsfba", 0);
    print_hash(224, (const unsigned char*)"sasbdfna", 0);
    print_hash(224, (const unsigned char*)"", 0);
    print_hash(224, (const unsigned char*)"", 0);
    print_hash(224, (const unsigned char*)"", 1);
    print_hash(224, (const unsigned char*)"\0", 1);
    unsigned char zero = '\0';
    print_hash(224, &zero, 1);
*/
}