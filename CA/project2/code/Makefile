FLAGS = -Wall

.PHONY: default run remake clean

default:
	iverilog $(FLAGS) -o testbench *.v

run:
	./testbench

remake: clean default

clean:
	rm -f testbench output.txt output.vcd
