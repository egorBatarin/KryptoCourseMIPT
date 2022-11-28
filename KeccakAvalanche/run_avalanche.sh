#!/bin/bash
# gen test data
#./avalanche_input.py 100 0 512 1
for h in 224 256 384 512
do
    for i in 1 2 3 4 8 12 24
    do
        ./avalanche_test.out $h $i 1
        ./plot.py
    done
done