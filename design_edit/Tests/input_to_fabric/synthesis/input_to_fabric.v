/* Generated by Yosys 0.38 (git sha1 d2189b06a, gcc 11.2.1 -fPIC -Os) */

module primitive_example_design_4(in, clk, rst, ibuf1_en, ibuf2_en, ibuf3_en, ibuf4_en, ibuf5_en, q_n, q_p);
  input clk;
  input ibuf1_en;
  input ibuf2_en;
  input ibuf3_en;
  input ibuf4_en;
  input ibuf5_en;
  input [2:0] in;
  output [2:0] q_n;
  output [2:0] q_p;
  input rst;
  wire \$iopadmap$clk ;
  (* unused_bits = "0" *)
  wire \$iopadmap$ibuf1_en ;
  wire \$iopadmap$ibuf2_en ;
  wire \$iopadmap$ibuf3_en ;
  wire \$iopadmap$ibuf4_en ;
  wire \$iopadmap$ibuf5_en ;
  (* src = "./rtl/primitive_example_design_4.v:3.11-3.14" *)
  (* src = "./rtl/primitive_example_design_4.v:3.11-3.14" *)
  wire clk;
  (* src = "./rtl/primitive_example_design_4.v:13.38-13.49" *)
  wire clk_buf_out;
  (* src = "./rtl/primitive_example_design_4.v:11.16-11.25" *)
  wire [2:0] dffre_out;
  (* src = "./rtl/primitive_example_design_4.v:9.16-9.25" *)
  wire [2:0] i_buf_out;
  (* src = "./rtl/primitive_example_design_4.v:4.11-4.19" *)
  (* src = "./rtl/primitive_example_design_4.v:4.11-4.19" *)
  wire ibuf1_en;
  (* src = "./rtl/primitive_example_design_4.v:4.20-4.28" *)
  (* src = "./rtl/primitive_example_design_4.v:4.20-4.28" *)
  wire ibuf2_en;
  (* src = "./rtl/primitive_example_design_4.v:4.29-4.37" *)
  (* src = "./rtl/primitive_example_design_4.v:4.29-4.37" *)
  wire ibuf3_en;
  (* src = "./rtl/primitive_example_design_4.v:4.38-4.46" *)
  (* src = "./rtl/primitive_example_design_4.v:4.38-4.46" *)
  wire ibuf4_en;
  (* src = "./rtl/primitive_example_design_4.v:4.47-4.55" *)
  (* src = "./rtl/primitive_example_design_4.v:4.47-4.55" *)
  wire ibuf5_en;
  (* src = "./rtl/primitive_example_design_4.v:2.17-2.19" *)
  (* src = "./rtl/primitive_example_design_4.v:2.17-2.19" *)
  wire [2:0] in;
  (* src = "./rtl/primitive_example_design_4.v:5.18-5.21" *)
  (* src = "./rtl/primitive_example_design_4.v:5.18-5.21" *)
  wire [2:0] q_n;
  (* src = "./rtl/primitive_example_design_4.v:6.18-6.21" *)
  (* src = "./rtl/primitive_example_design_4.v:6.18-6.21" *)
  wire [2:0] q_p;
  (* src = "./rtl/primitive_example_design_4.v:3.16-3.19" *)
  (* src = "./rtl/primitive_example_design_4.v:3.16-3.19" *)
  wire rst;
  (* src = "./rtl/primitive_example_design_4.v:10.10-10.23" *)
  wire rst_i_buf_out;
  (* keep = 32'h00000001 *)
  (* module_not_derived = 32'h00000001 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/jul18/Raptor/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$iopadmap$primitive_example_design_4.clk  (
    .EN(1'h1),
    .I(clk),
    .O(\$iopadmap$clk )
  );
  (* keep = 32'h00000001 *)
  (* module_not_derived = 32'h00000001 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/jul18/Raptor/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$iopadmap$primitive_example_design_4.ibuf1_en  (
    .EN(1'h1),
    .I(ibuf1_en),
    .O(\$iopadmap$ibuf1_en )
  );
  (* keep = 32'h00000001 *)
  (* module_not_derived = 32'h00000001 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/jul18/Raptor/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$iopadmap$primitive_example_design_4.ibuf2_en  (
    .EN(1'h1),
    .I(ibuf2_en),
    .O(\$iopadmap$ibuf2_en )
  );
  (* keep = 32'h00000001 *)
  (* module_not_derived = 32'h00000001 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/jul18/Raptor/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$iopadmap$primitive_example_design_4.ibuf3_en  (
    .EN(1'h1),
    .I(ibuf3_en),
    .O(\$iopadmap$ibuf3_en )
  );
  (* keep = 32'h00000001 *)
  (* module_not_derived = 32'h00000001 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/jul18/Raptor/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$iopadmap$primitive_example_design_4.ibuf4_en  (
    .EN(1'h1),
    .I(ibuf4_en),
    .O(\$iopadmap$ibuf4_en )
  );
  (* keep = 32'h00000001 *)
  (* module_not_derived = 32'h00000001 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/jul18/Raptor/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$iopadmap$primitive_example_design_4.ibuf5_en  (
    .EN(1'h1),
    .I(ibuf5_en),
    .O(\$iopadmap$ibuf5_en )
  );
  (* module_not_derived = 32'h00000001 *)
  (* src = "./rtl/primitive_example_design_4.v:13.13-13.51" *)
  CLK_BUF clk_buf_inst (
    .I(\$iopadmap$clk ),
    .O(clk_buf_out)
  );
  (* module_not_derived = 32'h00000001 *)
  (* src = "./rtl/primitive_example_design_4.v:24.11-24.98" *)
  DFFRE ff_inst1 (
    .C(clk_buf_out),
    .D(in[0]),
    .E(1'h1),
    .Q(dffre_out[0]),
    .R(rst_i_buf_out)
  );
  (* module_not_derived = 32'h00000001 *)
  (* src = "./rtl/primitive_example_design_4.v:25.11-25.98" *)
  DFFRE ff_inst2 (
    .C(clk_buf_out),
    .D(i_buf_out[1]),
    .E(1'h1),
    .Q(dffre_out[1]),
    .R(rst_i_buf_out)
  );
  (* module_not_derived = 32'h00000001 *)
  (* src = "./rtl/primitive_example_design_4.v:26.11-26.98" *)
  DFFRE ff_inst3 (
    .C(clk_buf_out),
    .D(i_buf_out[2]),
    .E(1'h1),
    .Q(dffre_out[2]),
    .R(rst_i_buf_out)
  );
  (* module_not_derived = 32'h00000001 *)
  (* src = "./rtl/primitive_example_design_4.v:20.11-20.64" *)
  I_BUF ibuf_inst2 (
    .EN(\$iopadmap$ibuf3_en ),
    .I(in[1]),
    .O(i_buf_out[1])
  );
  (* module_not_derived = 32'h00000001 *)
  (* src = "./rtl/primitive_example_design_4.v:21.11-21.64" *)
  I_BUF ibuf_inst3 (
    .EN(\$iopadmap$ibuf4_en ),
    .I(in[2]),
    .O(i_buf_out[2])
  );
  (* module_not_derived = 32'h00000001 *)
  (* src = "./rtl/primitive_example_design_4.v:22.11-22.63" *)
  I_BUF ibuf_inst4 (
    .EN(\$iopadmap$ibuf5_en ),
    .I(rst),
    .O(rst_i_buf_out)
  );
  (* module_not_derived = 32'h00000001 *)
  (* src = "./rtl/primitive_example_design_4.v:15.14-15.72" *)
  O_BUF_DS obuf_ds_inst1 (
    .I(dffre_out[0]),
    .O_N(q_n[0]),
    .O_P(q_p[0])
  );
  (* module_not_derived = 32'h00000001 *)
  (* src = "./rtl/primitive_example_design_4.v:16.14-16.72" *)
  O_BUF_DS obuf_ds_inst2 (
    .I(dffre_out[1]),
    .O_N(q_n[1]),
    .O_P(q_p[1])
  );
  (* module_not_derived = 32'h00000001 *)
  (* src = "./rtl/primitive_example_design_4.v:17.14-17.72" *)
  O_BUF_DS obuf_ds_inst3 (
    .I(dffre_out[2]),
    .O_N(q_n[2]),
    .O_P(q_p[2])
  );
endmodule