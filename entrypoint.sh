#!/bin/bash

rm -rf Lab
cp -r Code Lab
cp Makefile_official Lab/Makefile
(cd Lab && make clean && make parser)
(cd compilers-tests/irsim && make clean)
./compilers-tests/run.sh ../Lab/parser
(cd Lab && make clean)
(cd compilers-tests/irsim && make clean)