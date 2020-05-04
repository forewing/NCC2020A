#!/bin/bash

rm -rf Lab/*
mkdir -p Lab

(cd Code && make parser)
mv Code/parser Lab/parser

cp report.pdf Lab/

(cd Code && make clean)
cp -r Code Lab/
cp Makefile_official Lab/Code/Makefile
rm -rf Lab/Code/parser-gdb*

mkdir -p Lab/Test
cp -r Test/Lab1 Lab/Test
cp -r Test/Lab2 Lab/Test

cp README Lab/

if [[ -z "$1" ]]; then
    TARGET_ZIP="Lab.zip"
else
    TARGET_ZIP="$1.zip"
fi

rm -rf *.zip
zip -q -r ${TARGET_ZIP} Lab
echo "Compressed into ${TARGET_ZIP}"
