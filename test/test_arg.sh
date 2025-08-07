#!/bin/bash
set -e
gcc -o test_arg.out test_arg.c ../src/rlarg.c \
    -Og -rdynamic -ggdb3 -lrlc -lrlso -fsanitize=address \

    #-Og -rdynamic -ggdb3 -lrlc -lrlso \

    #-lrphiic
    #-O3 -flto -march=native -mfpmath=sse \
    #Og -fsanitize=address -rdynamic -ggdb3 \

#/usr/bin/time -vvv ./test_arg.out $@ || true

./test_arg.out $@ || true
rm test_arg.out

