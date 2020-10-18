module ID_EX
  (
   input 	     clk_i,
   input [31:0]      inst_i,
   input [31:0]      pc_i,
   input [31:0]      RDData0_i,
   input [31:0]      RDData1_i,
   input [31:0]      SignExtended_i,
   input stall_i,
   output [31:0] RDData0_o,
   output [31:0] RDData1_o,
   output [31:0] SignExtended_o,
   output [31:0] inst_o ,
   output [31:0] pc_o,
		 //control
   input 	     RegDst_i,
   input [1:0] 	     ALUOp_i,
   input 	     ALUSrc_i,
   input 	     RegWrite_i,
   input 	     MemToReg_i,
   input 	     MemRead_i,
   input 	     MemWrite_i,
   output 	     RegDst_o,
   output [1:0]      ALUOp_o,
   output 	     ALUSrc_o,
   output 	     RegWrite_o,
   output 	     MemToReg_o,
   output 	     MemRead_o,
   output 	     MemWrite_o
   );

   reg [31:0] RDData0_o = 32'd0;
   reg [31:0] RDData1_o = 32'd0;
   reg [31:0] SignExtended_o = 32'd0;
   reg [31:0] inst_o = 32'd0;
   reg [31:0] pc_o = 32'd0;



   // Control
   reg 		 RegDst_r =0;
   reg [1:0] 	 ALUOp_r=2'd0;
   reg 		 ALUSrc_r=0;
   reg 		 RegWrite_r=0;
   reg 		 MemToReg_r=0;
   reg 		 MemRead_r=0;
   reg 		 MemWrite_r=0;

   assign RegDst_o = RegDst_r;
   assign ALUOp_o = ALUOp_r;
   assign ALUSrc_o = ALUSrc_r;
   assign RegWrite_o = RegWrite_r;
   assign MemToReg_o = MemToReg_r;
   assign MemRead_o = MemRead_r;
   assign MemWrite_o = MemWrite_r;



   always@(posedge clk_i) begin
     if (stall_i) begin
     end
     else begin
      RDData0_o <= RDData0_i;
      RDData1_o <= RDData1_i;
      SignExtended_o <= SignExtended_i;
      pc_o <= pc_i;
      inst_o <= inst_i;
      //control
      RegDst_r <= RegDst_i;
      ALUOp_r <= ALUOp_i;
      ALUSrc_r <= ALUSrc_i;
      RegWrite_r <= RegWrite_i;
      MemToReg_r <= MemToReg_i;
      MemRead_r <= MemRead_i;
      MemWrite_r <= MemWrite_i;
    end
   end

endmodule
