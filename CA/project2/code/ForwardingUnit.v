module Forwarding
(
  input EX_MEM_RegWrite,
  input MEM_WB_RegWrite,
  input  [4:0] EX_MEM_RegisterRd,
  input  [4:0] MEM_WB_RegisterRd,
  input  [4:0] ID_EX_RegisterRs,
  input  [4:0] ID_EX_RegisterRt,
  output [1:0] ForwardA,
  output [1:0] ForwardB
);

reg [1:0] Forwarda;
reg [1:0] Forwardb;

assign ForwardA = Forwarda;
assign ForwardB = Forwardb;

always @(*) begin
  Forwarda = 2'b00;
  Forwardb = 2'b00;
  /*  EX Hazard */
  if (EX_MEM_RegWrite &&
      EX_MEM_RegisterRd != 5'b0 &&
      EX_MEM_RegisterRd == ID_EX_RegisterRs)
      Forwarda = 2'b10;
  if (EX_MEM_RegWrite &&
      EX_MEM_RegisterRd != 5'b0 &&
      EX_MEM_RegisterRd == ID_EX_RegisterRt)
      Forwardb = 2'b10;
  /*  Mem Hazard */
  if (MEM_WB_RegWrite &&
      MEM_WB_RegisterRd != 5'b0 &&
      //!(EX_MEM_RegWrite && (EX_MEM_RegisterRd != 0) && (EX_MEM_RegisterRd !=ID_EX_RegisterRs)) &&
      MEM_WB_RegisterRd == ID_EX_RegisterRs)
      Forwarda = 2'b01;
  if (MEM_WB_RegWrite &&
      MEM_WB_RegisterRd != 5'b0 &&
      //!(EX_MEM_RegWrite && (EX_MEM_RegisterRd != 0) && (EX_MEM_RegisterRd != ID_EX_RegisterRt)) &&
      MEM_WB_RegisterRd == ID_EX_RegisterRt)
      Forwardb = 2'b01;
end

endmodule
