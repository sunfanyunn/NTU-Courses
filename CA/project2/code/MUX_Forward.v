module MUX_Forward (
	input [31:0] data0_i,
	input [31:0] data1_i,
	input [31:0] data2_i,
	input [1:0] IsForward_i,
	output [31:0] data_o
);

assign data_o = (
	(IsForward_i == 2'b00)? data0_i :
	(IsForward_i == 2'b01)? data1_i :
	(IsForward_i == 2'b10)? data2_i :
	data0_i
);

endmodule
