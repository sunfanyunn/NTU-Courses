module EX_MEM
  (
   input 	     clk_i,
   input [31:0]      pc_i,
   input 	     zero_i,
   input [31:0]      ALUResult_i,
   input [31:0]      RDData_i,
   input [4:0] 	     RDaddr_i,
   input stall_i,

   output [31:0] pc_o,
   output        zero_o,
   output [31:0] ALUResult_o,
   output [31:0] RDData_o,
   output [4:0]  RDaddr_o,
   
   //control
   input 	     RegWrite_i,
   input 	     MemToReg_i,
   input 	     MemRead_i,
   input 	     MemWrite_i,
   output 	     RegWrite_o,
   output 	     MemToReg_o,
   output 	     MemRead_o,
   output 	     MemWrite_o
   );

   reg [31:0] pc_r = 32'd0;
   reg        zero_r = 1'b0;
   reg [31:0] ALUResult_r = 32'd0;
   reg [31:0] RDData_r = 32'd0;
   reg [4:0]  RDaddr_r = 5'd0;

   assign pc_o = pc_r;
   assign zero_o = zero_r;
   assign ALUResult_o = ALUResult_r;
   assign RDData_o = RDData_r;
   assign RDaddr_o = RDaddr_r;

   reg 		     RegWrite_r=0;
   reg 		     MemToReg_r=0;
   reg 		     MemRead_r=0;
   reg 		     MemWrite_r=0;

   assign RegWrite_o = RegWrite_r;
   assign MemToReg_o = MemToReg_r;
   assign MemRead_o = MemRead_r;
   assign MemWrite_o = MemWrite_r;

   always@(posedge clk_i) begin
     if (stall_i) begin
     end
     else begin
      pc_r <= pc_i;
      zero_r <= zero_i;
      RDaddr_r <= RDaddr_i;
      ALUResult_r <= ALUResult_i;
      RDData_r <= RDData_i;
      //control
      RegWrite_r <= RegWrite_i;
      MemToReg_r <= MemToReg_i;
      MemRead_r <= MemRead_i;
      MemWrite_r <= MemWrite_i;
    end
   end

endmodule
