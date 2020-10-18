module Control (
  input [5:0] Op_i,
  output RegDst_o,
  output [1:0] ALUOp_o,
  output ALUSrc_o,
  output RegWrite_o,
  output MemToReg_o,
  output MemRead_o,
  output MemWrite_o,
  output IsBranch_o,
  output IsJump_o
);

parameter R_TYPE = 6'b000000; // add, sub, mul, and, or
parameter ADDI= 6'b001000;
parameter LW  = 6'b100011;
parameter SW  = 6'b101011;
parameter BEQ = 6'b000100;
parameter JMP = 6'b000010;

// "don't care" is set to 0
assign RegDst_o = (Op_i == R_TYPE);
assign ALUSrc_o = (Op_i == ADDI) | (Op_i == LW) | (Op_i == SW);
assign MemToReg_o = (Op_i == LW);
assign RegWrite_o = (Op_i == R_TYPE) | (Op_i == ADDI) | (Op_i == LW);
assign MemRead_o = (Op_i == LW);
assign MemWrite_o = (Op_i == SW);
assign IsBranch_o = (Op_i == BEQ);
assign IsJump_o = (Op_i == JMP);

assign ALUOp_o = (
  (Op_i == ADDI || Op_i == LW || Op_i == SW) ? ALU_Control.ADD :
  (Op_i == BEQ) ? ALU_Control.SUB :
  ALU_Control.R_TYPE
);

endmodule
