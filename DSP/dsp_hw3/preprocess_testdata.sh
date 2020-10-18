DIRNAME=test
mkdir -p $DIRNAME

for i in $(seq 1 10) ; do
  ./separator_big5.pl testdata/$i.txt > $DIRNAME/$i.txt
done;

./separator_big5.pl testdata/example.txt > $DIRNAME/example.txt

rm testdata -rf
mv $DIRNAME testdata


