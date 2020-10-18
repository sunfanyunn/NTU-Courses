`timescale 1 ns/ 1 ns

module alu (src_a, src_b, c, data_out);

input [7:0] src_a, src_b;
input [2:0] c;
output [7:0] data_out;

reg[7:0] out;
assign data_out = out;

always @(*) begin
  case(c)
    1: out = src_a + src_b;
    2: out = src_a - src_b;
    3: out = src_a & src_b;
    4: out = src_a | src_b;
    5: out = src_a ^ src_b;
    default: out = src_a;
  endcase
end

endmodule
