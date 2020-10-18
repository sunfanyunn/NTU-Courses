module PC
(
	clk_i,
    rst_i,
	start_i,
	stall_i,
	pcEnable_i,
	pc_i,
	pc_o
);

// Interface
input				   clk_i;
input				   rst_i;
input				   start_i;
input				   stall_i;
input          pcEnable_i;
input	 [31:0]		pc_i;
output	[31:0]		pc_o;

// Signals
reg		[31:0]		pc_o;

always@(posedge clk_i or negedge rst_i) begin
    if(~rst_i) begin
        pc_o <= 32'b0;
    end
    else begin
    	if(stall_i) begin
    	end
    	else if(start_i)	begin
    		if( pcEnable_i )
    			pc_o <= pc_i;
    	end
    	else
    		pc_o <= 32'b0;
    end
end

endmodule
