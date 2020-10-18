module CPU
(
	clk_i,
	rst_i,
	start_i,
   
	mem_data_i, 
	mem_ack_i, 	
	mem_data_o, 
	mem_addr_o, 	
	mem_enable_o, 
	mem_write_o
);

//input
input clk_i;
input rst_i;
input start_i;

//
// to Data Memory interface		
//
input	[256-1:0]	mem_data_i; 
input				mem_ack_i; 	
output	[256-1:0]	mem_data_o; 
output	[32-1:0]	mem_addr_o; 	
output				mem_enable_o; 
output				mem_write_o; 

//
// add your project1 here!
//


PC PC
(
	.clk_i(clk_i),
	.rst_i(rst_i),
	.start_i(start_i),
	.stall_i(),
	.pcEnable_i(),
	.pc_i(),
	.pc_o()
);

Instruction_Memory Instruction_Memory(
	.addr_i(), 
	.instr_o()
);

Registers Registers(
	.clk_i(clk_i),
	.RSaddr_i(),
	.RTaddr_i(),
	.RDaddr_i(), 
	.RDdata_i(),
	.RegWrite_i(), 
	.RSdata_o(), 
	.RTdata_o() 
);

//data cache
dcache_top dcache
(
    // System clock, reset and stall
	.clk_i(clk_i), 
	.rst_i(rst_i),
	
	// to Data Memory interface		
	.mem_data_i(mem_data_i), 
	.mem_ack_i(mem_ack_i), 	
	.mem_data_o(mem_data_o), 
	.mem_addr_o(mem_addr_o), 	
	.mem_enable_o(mem_enable_o), 
	.mem_write_o(mem_write_o), 
	
	// to CPU interface	
	.p1_data_i(), 
	.p1_addr_i(), 	
	.p1_MemRead_i(), 
	.p1_MemWrite_i(), 
	.p1_data_o(), 
	.p1_stall_o()
);

endmodule