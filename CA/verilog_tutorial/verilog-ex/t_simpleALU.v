// Code your testbench here
// or browse Examples
`timescale 1 ns/1 ns

module testbench;
reg [7:0] src_a, src_b;
reg [2:0] c;
wire [7:0] data_out;

alu ALU_instance(.src_a(src_a), .src_b(src_b), .c(c), .data_out(data_out) );

initial
begin
  src_a = 8'h55;  // Time = 0
  src_b = 8'h1a;
  c	= 3'b000;
  #50;		  // Time = 50
  c	= 3'b001; 
  #50;		  // Time = 100 
  c	= 3'b010; 
  #50;		  // Time = 150 
  c	= 3'b011; 
  #50;		  // Time = 200 
  c	= 3'b100; 
  #50;		  // Time = 250 
  c	= 3'b101; 
  #50;		  // Time = 300 
  c	= 3'b110; 
  #50;		  // Time = 350
  c	= 3'b111; 
 
end
  
always #50 begin
  $monitor("time: %8d, a:%d, b:%d, c:%d, d_out:%d",$time, src_a, src_b, c,data_out);
end

initial #400 $finish;
endmodule
