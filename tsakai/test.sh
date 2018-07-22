#! /bin/bash
rm *.txt
mkdir test
g++ gen.cpp 
mv ./a.out test

cd test
./a.out
rm a.out
cd ../

make 
mv a.out test
cd test
files="*.txt"
for filepath in $files; do
    echo =====
    echo $filepath
    ./a.out $filepath $filepath.result
    cat $filepath.result
done
