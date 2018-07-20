#! /bin/bash
g++ gen.cpp
./a.out
make 
files="*.txt"
for filepath in $files; do
    ./a.out $filepath
done
