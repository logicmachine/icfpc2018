#!/bin/bash

cd dataset 

for filename in $( ls . | grep .mdl$ ); do
    #echo "${filename}" "${filename%.mdl}_ans.nbt"
    #echo "${filename}" "${filename%.mdl}_ans.nbt" | ./a.out `cat`
    echo ${filename} ../output/${filename%.mdl}_ans.nbt
    ../a.out ${filename} ../output/${filename%.mdl}_ans.nbt
done
