module MUX5 (
  input [4:0] data0_i,
  input [4:0] data1_i,
  input select_i,
  output [4:0] data_o
);

assign data_o = select_i ? data1_i : data0_i;

endmodule
