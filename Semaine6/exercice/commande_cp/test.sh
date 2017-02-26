#! /bin/bash

make test

./a.out toto tata

S1=$(sha1sum toto | cut -d' ' -f1)
S2=$(sha1sum tata | cut -d' ' -f1)

echo $S1
echo $S2

if [ $S1 = $S2 ]
then
	echo OK
else
	echo KO
fi
