FLAGS=-O3
main: main.cpp 
	g++ -std=c++11  $(FLAGS) main.cpp -o main
debug: 
	g++ -std=c++11  $(FLAGS) -D DEBUG -g  main.cpp -o main
run: main
	./main 
clean:
	rm -rf main *.o
