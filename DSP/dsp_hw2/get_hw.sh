#!/bin/bash
set -x

DIRNAME=b04902045

rm -rf tmp/$DIRNAME
mkdir -p tmp/$DIRNAME

# Copy Source Files
cp 0*.sh tmp/$DIRNAME
cp lib/proto tmp/$DIRNAME
cp lib/mix2_10.hed tmp/$DIRNAME

# run everything
./go.sh 

# Print Accuracy
cat result/accuracy
mv result/accuracy tmp/$DIRNAME/accuracy

# Export Report as PDF
gimli -file README.md -outputdir tmp/$DIRNAME -outputfilename hw2-1_b04902045

# Archive
cd tmp
zip -r $DIRNAME.zip $DIRNAME
cd ..
