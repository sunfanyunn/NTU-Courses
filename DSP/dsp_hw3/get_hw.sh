DIRNAME=dsp_hw3_b04902045

cp Makefile report.pdf $DIRNAME
cp mappig.py mydisambig.cpp $DIRNAME
cp -r result1/ $DIRNAME

zip -r $DIRNAME.zip $DIRNAME
