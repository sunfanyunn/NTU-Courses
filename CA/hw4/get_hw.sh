#!/bin/bash
set -ex

# test
mv instruction.txt go.sh src && cd src
./go.sh
diff output.txt ../answer && rm output.txt a.out
mv instruction.txt go.sh ../ && cd ../

DIRNAME=hw4_b04902045_v0

rm -rf tmp/$DIRNAME
mkdir -p tmp/$DIRNAME

cp -r src tmp/$DIRNAME
cp instruction.txt tmp/$DIRNAME/src


gimli -file README.md -outputdir tmp/$DIRNAME -outputfilename hw4_b04902045

cd tmp
zip -r $DIRNAME.zip $DIRNAME
