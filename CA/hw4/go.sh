#!/bin/bash 
set -ex
iverilog testbench.v \
         CPU.v \
         Instruction_Memory.v \
         PC.v \
         Registers.v \
         MUX32.v \
         MUX5.v \
         Sign_Extend.v \
         Control.v \
         ALU_Control.v \
         Adder.v \
         ALU.v 

vvp a.out
