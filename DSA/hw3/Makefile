FLAGS=-O3
all: main.cpp 
	g++ -std=c++11  $(FLAGS) main.cpp -o main

debug: 
	g++ -std=c++11 $(FLAGS) -D DEBUG main.cpp -o main
run: main
	./main
clean:
	rm -rf main *.o
