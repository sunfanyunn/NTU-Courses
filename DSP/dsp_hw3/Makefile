# The following two variable will be commandline determined by TA
# For testing, you could uncomment them.
SRIPATH ?= ./srilm-1.5.10
MACHINE_TYPE ?= i686-m64
LM ?= bigram.lm

CXX = g++ -std=c++11 -O3
CXXFLAGS = -O3 -I$(SRIPATH)/include -w
vpath lib%.a $(SRIPATH)/lib/$(MACHINE_TYPE)

TARGET = mydisambig
SRC = mydisambig.cpp
OBJ = $(SRC:.cpp=.o)
TO = ZhuYin-Big5.map
FROM = Big5-ZhuYin.map
.PHONY: all clean map run

TEST1DIR=result1
TESTDATA=testdata
ANSDIR=result2

all: $(TARGET)

$(TARGET): $(OBJ) -loolm -ldstruct -lmisc
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

run:
	@#TODO How to run your code toward different txt? 
	[ -d $(ANSDIR) ] || mkdir -p $(ANSDIR);
	@for i in $(shell seq 1 10) ; do \
            echo "Running $$i.txt"; \
            ./mydisambig -text testdata/$$i.txt -map $(TO) -lm $(LM) -order 2 > result2/$$i.txt; \
        done;
map:
	@#TODO How to map?
	echo "Mapping!"
	python3 mapping.py $(FROM) $(TO)

test1: 
	[ -d $(TEST1DIR) ] || mkdir -p $(TEST1DIR);
	for i in $(shell seq 1 10); do \
		$(SRIPATH)/bin/i686-m64/disambig -text testdata/$${i}.txt -map $(TO) -lm $(LM) -order 2 > $(TEST1DIR)/$${i}.txt; \
	done;

clean:
	$(RM) $(OBJ) $(TARGET)

cleann:
	$(RM) -r $(OBJ) $(TARGET) $(ANSDIR) $(TO) corpus_seg.txt lm.cnt 
