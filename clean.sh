#! /bin/sh
find . -name CMakeFiles | xargs rm -rf
find . -name cmake_install.cmake | xargs rm -rf
find . -name CMakeCache.txt | xargs rm -rf
find . -name Makefile | xargs rm -rf
rm -rf build




