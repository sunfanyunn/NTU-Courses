module IF_ID (
  input clk_i,
  input [31:0] inst_i,
  input [31:0] pc_i,
  input hazard_in,
  input flush,
  input stall_i,
  output reg [31:0] inst_o = 32'd0,
  output reg [31:0] pc_o = 32'd0
);

always @(posedge clk_i) begin
  if (stall_i) begin
  end
  else begin
    pc_o <= pc_i;
    inst_o <= (
      (flush)? 32'b0 : // flush
      (hazard_in)? inst_o : // stall
      inst_i
    );
  end
end

endmodule
