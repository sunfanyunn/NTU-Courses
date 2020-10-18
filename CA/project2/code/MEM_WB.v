module MEM_WB
  (
   input 	     clk_i,
   input [31:0]      RDData_i,
   input [31:0]      ALUResult_i,
   input [4:0]      RDaddr_i,
   input stall_i,

   output [4:0] RDaddr_o,
   output [31:0] RDData_o,
   output [31:0] ALUResult_o,

		     //control
   output 	     RegWrite_o,
   output 	     MemToReg_o,
   input 	     RegWrite_i,
   input 	     MemToReg_i
   );

   output reg [4:0] RDaddr_r=5'd0;
   output reg [31:0] RDData_r=32'd0;
   output reg [31:0] ALUResult_r=32'd0;

   assign RDaddr_o = RDaddr_r;
   assign RDData_o = RDData_r;
   assign ALUResult_o = ALUResult_r;

   // Control
   reg 		    RegWrite_r=0;
   reg 		    MemToReg_r=0;

   assign RegWrite_o = RegWrite_r;
   assign MemToReg_o = MemToReg_r;

   always@(posedge clk_i) begin
     if (stall_i) begin
     end
     else begin
      RDData_r <= RDData_i;
      RDaddr_r <= RDaddr_i;
      ALUResult_r <= ALUResult_i;

      //control
      RegWrite_r <= RegWrite_i;
      MemToReg_r <= MemToReg_i;
    end
   end

endmodule
