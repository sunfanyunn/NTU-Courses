module MUX32 (
  input [31:0] data0_i,
  input [31:0] data1_i,
  input select_i,
  output [31:0] data_o
);

assign data_o = select_i ? data1_i : data0_i;

endmodule
