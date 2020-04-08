#!/bin/bash

rm -rf Lab/*
mkdir -p Lab

(cd Code && make parser)
mv Code/parser Lab/parser

cp report.pdf Lab/

(cd Code && make clean)
cp -r Code Lab/
rm -rf Lab/Code/compilers-tests

mkdir -p Lab/Test
cp -r Test/Lab1 Lab/Test
cp -r Test/Lab2 Lab/Test

cp README Lab/

if [[ -z "${STUID}" ]]; then
    TARGET_ZIP="Lab.zip"
else
    TARGET_ZIP="${STUID}.zip"
fi

rm -rf *.zip
zip -q -r ${TARGET_ZIP} Lab
echo "Compressed into ${TARGET_ZIP}"
