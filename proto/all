#!/bin/sh
cd `pwd`
for i in `ls *proto`; do echo $i|awk -F'.' '{if($1!="Admin" && $1!="ProtoArchive" && $1!="ProtoUser") print "./m.sh "$1}'|bash;done
