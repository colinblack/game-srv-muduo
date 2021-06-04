#!/bin/sh
cd `pwd`
protoc -I=. --cpp_out=. $1.proto
