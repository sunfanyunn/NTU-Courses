module ALU 
(
  data0_i,
  data1_i,
  aluctrl_i,
  data_o,
  zero_o
);

// Interface
input [31:0] data0_i;
input [31:0] data1_i;
input [2:0] aluctrl_i;
output [31:0] data_o;
output zero_o;

parameter ADD = 3'b000;
parameter SUB = 3'b001; 
parameter MUL = 3'b010;
parameter AND = 3'b011;
parameter OR  = 3'b100;
parameter INVALID_OP = 3'bzzz;

assign data_o = (
  (aluctrl_i == ADD) ? data0_i + data1_i : 
  (aluctrl_i == SUB) ? data0_i - data1_i : 
  (aluctrl_i == MUL) ? data0_i * data1_i : 
  (aluctrl_i == AND) ? data0_i & data1_i : 
  (aluctrl_i == OR) ? data0_i | data1_i : 
  INVALID_OP
);
assign zero_o = (data_o == 0) ? 1 : 0;

endmodule
