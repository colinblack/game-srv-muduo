#!/bin/sh
cd `pwd`
protoc -I=. --cpp_out=. $1.proto
Proto2AS $1.proto
mkdir $1
mv -f *.as $1
rm -rf ../as/$1
mv -f $1/ ../as
cp $1.proto ../as
