#!/bin/bash

source full.conf

echo $targetDir
echo $privateID
echo $zipFileName


# check FA
fg=1
nums=`seq -f %03g 1 186`
for num in $nums; do
    filepath=$targetDir/FA$num.nbt
    if [ ! -e $filepath ]; then
	echo "Error: FA" $filepath "does not Exist"
	fg=0
    fi
done

# check FD (186)
nums=`seq -f %03g 1 186`
for num in $nums; do
    filepath=$targetDir/FD$num.nbt
    if [ ! -e $filepath ]; then
	echo "Error: FD" $filepath "does not Exist"
	fg=0
    fi
done


# check FR (115)
nums=`seq -f %03g 1 115`
for num in $nums; do
    filepath=$targetDir/FD$num.nbt
    if [ ! -e $filepath ]; then
	echo "Error: FD" $filepath "does not Exist"
	fg=0
    fi
done

if [ $fg -eq 0 ];then
   echo "Some files doesn't exist"
   exit 1
fi


zip -j -e --password=$privateID $zipFileName $targetDir/*.nbt
shaid=`shasum -a 256 $zipFileName | cut -f 1 -d ' '`

echo "sha256 checksum"
echo $shaid
