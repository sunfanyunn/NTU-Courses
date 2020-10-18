`timescale 1 ns/1 ns

module t_Full_Adder;

reg	a, b, ci;
wire	sum, cout;

Full_Adder FA_1(.a(a), .b(b), .ci(ci), .sum(sum), .cout(cout));

initial begin

  $dumpfile("Full_Adder.vcd");
  $dumpvars;
  
  // Time = 0
  a  = 1'b0;  
  b  = 1'b0;
  ci = 1'b0;

  $monitor("Time %8d ns, a=%d b=%d ci=%d sum=%d cout=%d", $time, a, b, ci, sum, cout);
end

always #50 begin
  a  = a + 1;
end

always #100 begin
  b  = b + 1;
end

always #200 begin
  ci  = ci + 1;
end

initial #1000 $stop;

endmodule
