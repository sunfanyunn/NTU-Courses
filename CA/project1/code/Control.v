module Control
(
  op_i,
  regdst_o,
  aluop_o,
  alusrc_o,
  regwrite_o,
);

// Interface
input [5:0] op_i;
output regdst_o;
output [1:0] aluop_o;
output alusrc_o;
output regwrite_o;

assign is_itype = op_i[3];
assign alusrc_o = is_itype;
assign regdst_o = ~ is_itype;
assign aluop_o = is_itype ? ALU_Control.ADDI : 0;
assign regwrite_o = 1;

endmodule
