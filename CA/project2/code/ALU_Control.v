module ALU_Control (
  input [5:0] funct_i,
  input [1:0] ALUOp_i,
  output [2:0] ALUCtrl_o
);

parameter ADD_FUNCT = 6'b100000;
parameter SUB_FUNCT = 6'b100010;
parameter MUL_FUNCT = 6'b011000;
parameter AND_FUNCT = 6'b100100;
parameter OR_FUNCT = 6'b100101;

wire [2:0] alu_r_type_op;
assign alu_r_type_op = (
  (funct_i == ADD_FUNCT) ? ALU.ADD :
  (funct_i == SUB_FUNCT) ? ALU.SUB :
  (funct_i == MUL_FUNCT) ? ALU.MUL :
  (funct_i == AND_FUNCT) ? ALU.AND :
  (funct_i == OR_FUNCT) ? ALU.OR :
  ALU.INVALID_OP
);

parameter ADD = 2'b00;
parameter SUB = 2'b01;
parameter R_TYPE = 2'b11;

assign ALUCtrl_o = (
  (ALUOp_i == R_TYPE) ? alu_r_type_op :
  (ALUOp_i == ADD) ? ALU.ADD :
  (ALUOp_i == SUB) ? ALU.SUB :
  ALU.INVALID_OP
);

endmodule
