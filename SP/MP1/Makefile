FLAGS=-O3
main: main.cpp 
	g++ -std=c++14  $(FLAGS) main.cpp -o change_count 
debug: 
	g++ -std=c++14  $(FLAGS) -D DEBUG -g  main.cpp -o main
run: main
	./main 
clean:
	rm -rf main *.o
