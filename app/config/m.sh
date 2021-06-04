#!/bin/sh
cd `pwd`
protoc -I=. --cpp_out=. $1.proto
mv $1.pb.cc $1.pb.cpp
