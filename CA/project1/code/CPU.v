module CPU
(
    clk_i, 
    rst_i,
    start_i
);

// Ports
input               clk_i;
input               rst_i;
input               start_i;

wire [31:0] ins;
wire [31:0] pc;

parameter PC_FORWARD_NUM = 32'd4;

Control Control(
    .op_i       (ins[31:26]),
    .regdst_o   (MUX_RegDst.select_i),
    .aluop_o    (ALU_Control.aluop_i),
    .alusrc_o   (MUX_ALUSrc.select_i),
    .regwrite_o (Registers.RegWrite_i)
);

Adder Add_PC(
    .data0_in   (pc),
    .data1_in   (PC_FORWARD_NUM),
    .data_o     (PC.pc_i)
);

PC PC(
    .clk_i      (clk_i), 
    .rst_i      (rst_i),
    .start_i    (start_i),
    .pc_i       (),
    .pc_o       (pc)
);

Instruction_Memory Instruction_Memory(
    .addr_i     (pc), 
    .instr_o    (ins)
);

Registers Registers(
    .clk_i      (clk_i),
    .RSaddr_i   (ins[25:21]),
    .RTaddr_i   (ins[20:16]),
    .RDaddr_i   (), 
    .RDdata_i   (),
    .RegWrite_i (), 
    .RSdata_o   (ALU.data0_i), 
    .RTdata_o   (MUX_ALUSrc.data0_i) 
);
  
MUX5 MUX_RegDst(
    .data0_i    (ins[20:16]),
    .data1_i    (ins[15:11]),
    .select_i   (),
    .data_o     (Registers.RDaddr_i)
);

MUX32 MUX_ALUSrc(
    .data0_i    (),
    .data1_i    (),
    .select_i   (),
    .data_o     (ALU.data1_i)
);

Sign_Extend Sign_Extend(
    .data_i     (ins[15:0]),
    .data_o     (MUX_ALUSrc.data1_i)
);
  
ALU ALU(
    .data0_i    (),
    .data1_i    (),
    .aluctrl_i  (),
    .data_o     (Registers.RDdata_i),
    .zero_o     ()
);

ALU_Control ALU_Control(
    .funct_i    (ins[5:0]),
    .aluop_i    (),
    .aluctrl_o  (ALU.aluctrl_i)
);

endmodule
