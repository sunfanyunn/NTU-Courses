module MUX8 (
	input IsHazzard_i,
	
	input RegDst_i, // EX
	input [1:0] ALUOp_i, // EX
	input ALUSrc_i, // EX
	input RegWrite_i, // WB
	input MemToReg_i, // WB
	input MemRead_i, // M
	input MemWrite_i, // M

	output RegDst_o,
	output [1:0] ALUOp_o,
	output ALUSrc_o,
	output RegWrite_o,
	output MemToReg_o,
	output MemRead_o,
	output MemWrite_o
);

assign RegDst_o   = (IsHazzard_i)? 1'b0 : RegDst_i;
assign ALUOp_o    = (IsHazzard_i)? 2'b00 : ALUOp_i;
assign ALUSrc_o   = (IsHazzard_i)? 1'b0 : ALUSrc_i;
assign RegWrite_o = (IsHazzard_i)? 1'b0 : RegWrite_i;
assign MemToReg_o = (IsHazzard_i)? 1'b0 : MemToReg_i;
assign MemRead_o  = (IsHazzard_i)? 1'b0 : MemRead_i;
assign MemWrite_o = (IsHazzard_i)? 1'b0 : MemWrite_i;

endmodule
