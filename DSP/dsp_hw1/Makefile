.PHONY: all clean

override CXXFLAGS+=-O3 -std=c++14 -Wall -Wextra -Wshadow -Wno-unused-result

TARGET=train test test_hmm
all: $(TARGET)

clean:
	$(RM) $(TARGET)   # type make clean to remove the compiled file
