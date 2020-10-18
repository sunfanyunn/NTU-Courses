module Full_Adder(sum, cout, a, b, ci);

//Interface
input	a, b, ci;
output	sum, cout;

// Calculation
assign {cout,sum} = a + b + ci;

endmodule
