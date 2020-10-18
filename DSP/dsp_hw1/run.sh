#!/bin/bash -ex
make
MIN=1
MAX=1000
file=curve

if [ -f $file ] ; then
  rm $file
fi

for ((i = MIN; i <= MAX; i+=1)); do 
  for ((j = 1; j <= 5; j++)); do 
    ./train $i data/model_init.txt data/seq_model_0"$j".txt model_0"$j".txt
  done
  ./test modellist.txt data/testing_data1.txt result1.txt
  python3 calc_acc.py > process
done

make clean
