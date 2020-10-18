module CPU (
  input clk_i,
  input rst_i,
  input start_i,
  // For cache
  input  [256-1:0] mem_data_i,
  input            mem_ack_i,
  output [256-1:0] mem_data_o,
  output [32-1:0]  mem_addr_o,
  output           mem_enable_o,
  output           mem_write_o
);

parameter PC_ADVANCE_NUM = 32'd4;
wire registers_equal = (Registers.RSdata_o == Registers.RTdata_o);
wire pc_src_branch_select = Control.IsBranch_o & registers_equal;

/* Flush START */
wire flush;
assign flush = Control.IsJump_o | pc_src_branch_select;
/* Flush END */

Control Control (
  .Op_i       (IF_ID.inst_o[31:26]),
  .RegDst_o   (),
  .ALUOp_o    (),
  .ALUSrc_o   (),
  .RegWrite_o (),
  .MemToReg_o (),
  .MemRead_o  (),
  .MemWrite_o (),
  .IsBranch_o (),
  .IsJump_o   ()
);

Adder Add_PCAdvance (
  .data0_i    (PC.pc_o),
  .data1_i    (PC_ADVANCE_NUM),
  .data_o     ()
);

Adder Add_PCBranch (
  .data0_i    (IF_ID.pc_o),
  .data1_i    (Shift_Left2_Branch.data_o),
  .data_o     ()
);

Shift_Left2 Shift_Left2_Branch (
  .data_i     (Sign_Extend.data_o),
  .data_o     ()
);

Shift_Left2 Shift_Left2_Jump (
  .data_i     (IF_ID.inst_o),
  .data_o     ()
);

Sign_Extend Sign_Extend (
  .data_i     (IF_ID.inst_o[15:0]),
  .data_o     ()
);

PC PC (
  .clk_i      (clk_i),
  .rst_i      (rst_i),
  .start_i    (start_i),
  .stall_i    (dcache.p1_stall_o),
  .pcEnable_i (HD_Unit.PC_Write),
  .pc_i       (MUX2.data_o),
  //.IsHazzard_i(HD_Unit.PC_Write),
  .pc_o       ()
);

Instruction_Memory Instruction_Memory (
  .addr_i     (PC.pc_o),
  .instr_o    ()
);

IF_ID IF_ID (
  .clk_i      (clk_i),
  .inst_i     (Instruction_Memory.instr_o),
  .pc_i       (Add_PCAdvance.data_o),
  .hazard_in  (HD_Unit.IF_ID_Write),
  .flush      (
    Control.IsJump_o |
    (Control.IsBranch_o &
      (Registers.RSdata_o == Registers.RTdata_o)
    )
  ),
  .stall_i    (dcache.p1_stall_o),
  .inst_o     (),
  .pc_o       ()
);

ID_EX ID_EX (
  .clk_i      (clk_i),
  .pc_i       (IF_ID.pc_o),
  .inst_i     (IF_ID.inst_o),
  .RDData0_i  (ID_Rs_Forward.data_o),
  .RDData1_i  (ID_Rt_Forward.data_o),
  .SignExtended_i(Sign_Extend.data_o),
  .stall_i    (dcache.p1_stall_o),
  .RDData0_o  (),
  .RDData1_o  (),
  .SignExtended_o(),
  .inst_o(),
  .pc_o(),

  //control
  .RegDst_i   (MUX8.RegDst_o),
  .ALUOp_i    (MUX8.ALUOp_o),
  .ALUSrc_i   (MUX8.ALUSrc_o),
  .RegWrite_i (MUX8.RegWrite_o),
  .MemToReg_i (MUX8.MemToReg_o),
  .MemRead_i  (MUX8.MemRead_o),
  .MemWrite_i (MUX8.MemWrite_o),
  .RegDst_o   (),
  .ALUOp_o    (),
  .ALUSrc_o   (),
  .RegWrite_o (),
  .MemToReg_o (),
  .MemRead_o  (),
  .MemWrite_o ()
);

EX_MEM EX_MEM (
  .clk_i      (clk_i),
  .pc_i       (ID_EX.pc_o),
  .zero_i     (ALU.zero_o),
  .ALUResult_i(ALU.data_o),
  .RDData_i   (MUX7.data_o),
  .RDaddr_i   (MUX3.data_o),
  .stall_i    (dcache.p1_stall_o),
  .pc_o       (),
  .zero_o     (),
  .ALUResult_o(),
  .RDData_o   (),
  .RDaddr_o   (),

  //control
  .RegWrite_o (),
  .MemToReg_o (),
  .MemRead_o  (),
  .MemWrite_o (),
  .RegWrite_i (ID_EX.RegWrite_o),
  .MemToReg_i (ID_EX.MemToReg_o),
  .MemRead_i  (ID_EX.MemRead_o),
  .MemWrite_i (ID_EX.MemWrite_o)
);

MEM_WB MEM_WB (
  .clk_i      (clk_i),
  //.RDData_i   (Data_Memory.RDdata_o),
  .RDData_i   (dcache.p1_data_o),
  .ALUResult_i(EX_MEM.ALUResult_o),
  .RDaddr_i   (EX_MEM.RDaddr_o),
  .stall_i    (dcache.p1_stall_o),
  .RDaddr_o   (),
  .RDData_o   (),
  .ALUResult_o(),

  //control
  .RegWrite_o (),
  .MemToReg_o (),
  .RegWrite_i (EX_MEM.RegWrite_o),
  .MemToReg_i (EX_MEM.MemToReg_o)
);

Registers Registers (
  .clk_i      (clk_i),
  .RSaddr_i   (IF_ID.inst_o[25:21]),
  .RTaddr_i   (IF_ID.inst_o[20:16]),
  .RDaddr_i   (MEM_WB.RDaddr_o),
  .RDdata_i   (MUX5.data_o),
  .RegWrite_i (MEM_WB.RegWrite_o),
  .RSdata_o   (),
  .RTdata_o   ()
);

ALU ALU (
  .data0_i    (MUX6.data_o),
  .data1_i    (MUX4.data_o),
  .aluctrl_i  (ALU_Control.ALUCtrl_o),
  .data_o     (),
  .zero_o     ()
);

ALU_Control ALU_Control (
  .funct_i    (ID_EX.inst_o[5:0]),
  .ALUOp_i    (ID_EX.ALUOp_o),
  .ALUCtrl_o  ()
);

//Memory Data_Memory (
//  .clk_i      (clk_i),
//  .RDaddr_i   (EX_MEM.ALUResult_o),
//  .RDdata_i   (EX_MEM.RDData_o),
//  .MemRead_i  (EX_MEM.MemRead_o),
//  .MemWrite_i (EX_MEM.MemWrite_o),
//  .RDdata_o   ()
//
//);

dcache_top dcache (
  // System clock, reset and stall
  .clk_i        (clk_i),
  .rst_i        (rst_i),

  // to Data Memory interface
  .mem_data_i   (mem_data_i),
  .mem_ack_i    (mem_ack_i),
  .mem_data_o   (mem_data_o),
  .mem_addr_o   (mem_addr_o),
  .mem_enable_o (mem_enable_o),
  .mem_write_o  (mem_write_o),

  // to CPU interface
  .p1_data_i    (EX_MEM.RDData_o),
  .p1_addr_i    (EX_MEM.ALUResult_o),
  .p1_MemRead_i (EX_MEM.MemRead_o),
  .p1_MemWrite_i(EX_MEM.MemWrite_o),
  .p1_data_o    (),
  .p1_stall_o   ()
);


Forwarding FW_Unit (
  .ID_EX_RegisterRs   (ID_EX.inst_o[25:21]),
  .ID_EX_RegisterRt   (ID_EX.inst_o[20:16]),
  .EX_MEM_RegisterRd  (EX_MEM.RDaddr_o), // mux3.data_o
  .MEM_WB_RegisterRd  (MEM_WB.RDaddr_o), // mux3.data_o

  // control
  .EX_MEM_RegWrite    (EX_MEM.RegWrite_o),
  .MEM_WB_RegWrite    (MEM_WB.RegWrite_o),
  .ForwardA           (),
  .ForwardB           ()
);

HazzardDetection HD_Unit (
  .IF_ID_RegisterRs (IF_ID.inst_o[25:21]),
  .IF_ID_RegisterRt (IF_ID.inst_o[20:16]),
  .ID_EX_RegisterRt (ID_EX.inst_o[20:16]),

  // control
  .ID_EX_MemRead_i  (ID_EX.MemRead_o), // ID_EX.MemRead_o
  .PC_Write         (),
  .IF_ID_Write      (),
  .data_o           () // for mux 8
);

MUX5 MUX3 (
  .data0_i    (ID_EX.inst_o[20:16]),
  .data1_i    (ID_EX.inst_o[15:11]),
  .select_i   (ID_EX.RegDst_o),
  .data_o     ()
);

MUX32 MUX1 (
  .data0_i    (Add_PCAdvance.data_o),
  .data1_i    (Add_PCBranch.data_o),
  .select_i   (pc_src_branch_select),
  .data_o     ()
);

MUX32 MUX2 (
  .data0_i    (MUX1.data_o),
  .data1_i    ({Add_PCAdvance.data_o[31:28], Shift_Left2_Jump.data_o[27:0]}),
  .select_i   (Control.IsJump_o),
  .data_o     ()
);

MUX32 MUX4 (
  .data0_i    (MUX7.data_o),
  .data1_i    (ID_EX.SignExtended_o),
  .select_i   (ID_EX.ALUSrc_o),
  .data_o     ()
);

MUX32 MUX5 (
  .data0_i    (MEM_WB.ALUResult_o),
  .data1_i    (MEM_WB.RDData_o),
  .select_i   (MEM_WB.MemToReg_o),
  .data_o     ()
);

MUX_Forward MUX6 (
  .data0_i      (ID_EX.RDData0_o), // ID_EX.RDdata0_out
  .data1_i      (MUX5.data_o), // from mux5 REG's result
  .data2_i      (EX_MEM.ALUResult_o), // from EX's result
  .data_o       (),

  // control
  .IsForward_i  (FW_Unit.ForwardA)
);

MUX_Forward MUX7 (
  .data0_i      (ID_EX.RDData1_o), // ID_EX.RDdata1_out
  .data1_i      (MUX5.data_o), // from mux5 REG's result
  .data2_i      (EX_MEM.ALUResult_o), // from EX's result
  .data_o       (),

  // control
  .IsForward_i  (FW_Unit.ForwardB)
);

MUX8 MUX8 (
  .IsHazzard_i  (HD_Unit.data_o),

  .RegDst_i     (Control.RegDst_o),
  .ALUOp_i      (Control.ALUOp_o),
  .ALUSrc_i     (Control.ALUSrc_o),
  .RegWrite_i   (Control.RegWrite_o),
  .MemToReg_i   (Control.MemToReg_o),
  .MemRead_i    (Control.MemRead_o),
  .MemWrite_i   (Control.MemWrite_o),

  .RegDst_o     (),
  .ALUOp_o      (),
  .ALUSrc_o     (),
  .RegWrite_o   (),
  .MemToReg_o   (),
  .MemRead_o    (),
  .MemWrite_o   ()
);

MUX32 ID_Rs_Forward (
  .data0_i    (Registers.RSdata_o),
  .data1_i    (MUX5.data_o),
  .select_i   (ID_FW_Unit.ForwardA),
  .data_o     ()
);

MUX32 ID_Rt_Forward (
  .data0_i    (Registers.RTdata_o),
  .data1_i    (MUX5.data_o),
  .select_i   (ID_FW_Unit.ForwardB),
  .data_o     ()
);


ID_Forwarding ID_FW_Unit (
  .IF_ID_RegisterRs   (IF_ID.inst_o[25:21]),
  .IF_ID_RegisterRt   (IF_ID.inst_o[20:16]),
  .MEM_WB_RegisterRd  (MEM_WB.RDaddr_o), // mux3.data_o
  // control
  .MEM_WB_RegWrite    (MEM_WB.RegWrite_o),
  .ForwardA           (),
  .ForwardB           ()
);

endmodule
