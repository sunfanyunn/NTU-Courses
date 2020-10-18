module ID_Forwarding (
	input MEM_WB_RegWrite,
	input [4:0] IF_ID_RegisterRs,
	input [4:0] IF_ID_RegisterRt,
	input [4:0] MEM_WB_RegisterRd,
	output ForwardA,
	output ForwardB
);

reg ForwardA, ForwardB;

always @(*) begin
	ForwardA = 0;
	ForwardB = 0;

	if (MEM_WB_RegWrite &&
		MEM_WB_RegisterRd != 5'b0 &&
		MEM_WB_RegisterRd == IF_ID_RegisterRs) begin
		ForwardA = 1;
	end

  if (MEM_WB_RegWrite &&
  	MEM_WB_RegisterRd != 5'b0 &&
  	MEM_WB_RegisterRd == IF_ID_RegisterRt) begin
  	ForwardB = 1;
  end
end

endmodule
