#!/bin/bash
set -x

DIRNAME=hw1_b04902045

rm -rf tmp/$DIRNAME
mkdir -p tmp/$DIRNAME

# Copy Source Files
cp train.cpp tmp/$DIRNAME
cp test.cpp tmp/$DIRNAME
cp Makefile tmp/$DIRNAME
cp run.sh tmp/$DIRNAME
cp calc_acc.py tmp/$DIRNAME
cp plot.py tmp/$DIRNAME

# Rebuild Program
make clean
make

# Run Train
ITER=960

./train $ITER data/model_init.txt data/seq_model_01.txt tmp/$DIRNAME/model_01.txt
./train $ITER data/model_init.txt data/seq_model_02.txt tmp/$DIRNAME/model_02.txt
./train $ITER data/model_init.txt data/seq_model_03.txt tmp/$DIRNAME/model_03.txt
./train $ITER data/model_init.txt data/seq_model_04.txt tmp/$DIRNAME/model_04.txt
./train $ITER data/model_init.txt data/seq_model_05.txt tmp/$DIRNAME/model_05.txt

# Run Test
cp data/modellist.txt tmp/model_list.txt
cd tmp/$DIRNAME
../../test ../model_list.txt ../../data/testing_data1.txt result1.txt
../../test ../model_list.txt ../../data/testing_data2.txt result2.txt
python ../../calc_acc.py result1.txt ../../data/testing_answer.txt > acc.txt
cd ../..

# Print Accuracy
cat tmp/$DIRNAME/acc.txt

# Export Report as PDF
gimli -file README.md -outputdir tmp/$DIRNAME -outputfilename Document

# Archive
cd tmp
zip -r $DIRNAME.zip $DIRNAME
cd ..
