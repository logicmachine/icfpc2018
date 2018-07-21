#!/bin/bash

source lightning.conf

echo $targetDir
echo $privateID
echo $zipFileName

nums=`seq -f %03g 1 186`
for num in $nums; do
    filepath=$targetDir/LA$num.nbt
    if [ ! -e $filepath ]; then
	echo "Error: " $filepath "does not Exist"
	exit 1
    fi
done

zip -j -e --password=$privateID $zipFileName $targetDir/*.nbt
shaid=`shasum -a 256 $zipFileName | cut -f 1 -d ' '`

echo "sha256 checksum"
echo $shaid
