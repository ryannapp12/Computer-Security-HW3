#!/bin/bash

rm -rf $1;

mkdir -p $1/{bin,mail,inputs,tmp}

while read -r i; do
    mkdir -p $1/mail/$i;
done < valid_usernames; 

cp *.test $1/inputs/. 

cp test-script $1/.

#!/bin/bash

make clean; 
make; 

mv mail-in $1/bin/mail-in 
mv mail-out $1/bin/mail-out

cp *.test $1/inputs/. 


for input in $1/inputs/*; do
    $1/bin/mail-in < $input;
done
