#!/usr/bin/env python3
from os import abort
import sys
import time
import random
import string
import argparse

ASCII = 8

def generate_random_str(length, seed=0):
    if seed > 0:
         random.seed(a=seed, version=2)
    return list(''.join(random.choices(string.ascii_letters, k=length)))

def generate_one_bit_diffs(origin_str, lines_limit=0, bits_per_sym=ASCII-1):
    str_list_lists = [origin_str + ['\n']]
    
    symbols_total = len(origin_str)
    lines = 1

    for sym_idx in range(symbols_total):
        for bit in range(bits_per_sym):
            copy_str = origin_str
            bit_mask = 1 << bit
            sym = copy_str[sym_idx]
            copy_str[sym_idx] = chr(ord(sym) ^ bit_mask)
            str_list_lists.append(copy_str + ['\n'])
            
            lines += 1
            if (lines_limit > 0 and lines >= lines_limit):
                return str_list_lists
    
    return str_list_lists

def main():
    # parse cmd args
    parser = argparse.ArgumentParser(description='Generates input for avalanche effect.')
    parser.add_argument('blockSizeLim', type=int, nargs='?', default=10,
                        help='max size of block')
    parser.add_argument('minLen', type=int, nargs='?', default=0,
                        help='min len of input string')
    parser.add_argument('maxLen', type=int, nargs='?', default=8,
                        help='max len of output string')
    parser.add_argument('lenStep', type=int, nargs='?', default=2,
                        help='step in length of input string')
    parser.add_argument('seed', type=int, nargs='?', default=1,
                        help='generate random strings constant seed')                    
                        
    args = parser.parse_args()
    print(args)
    
    minLen = args.minLen
    maxLen = args.maxLen
    lenStep = args.lenStep
    blockSizeLim = args.blockSizeLim
    seed = args.seed

    f = open('avalanche_effect.txt', 'w')
    
    if minLen == 0:
        #print("for len: 0 block: 1")
        print(file=f)
        minLen += lenStep

    #print("total blocks:", (maxLen - minLen + 1) / lenStep)
    for l in range(minLen, maxLen + 1, lenStep):

        str_variants = generate_one_bit_diffs(generate_random_str(l, seed), blockSizeLim)
        #print(str_variants)

        buffer = [item for sublist in str_variants for item in sublist]
        print(''.join(buffer), file=f, end='')

    f.close()

if __name__ == '__main__':
    main()