/* Generated by Yosys 0.18+10 (git sha1 8ecc445e4, gcc 11.4.0-1ubuntu1~22.04 -fPIC -Os) */

module and2(a, b, clk, reset, c, out);
  output out;
  input reset;
  output c;
  input b;
  input a;
  input [1:0] clk;
  (* src = "./Src/and2.v:9.9-9.10" *)
  wire \$auto$rs_design_edit.cc:682:execute$2.a ;
  (* keep = 32'd1 *)
  (* src = "./Src/and2.v:13.14-13.15" *)
  wire \$auto$rs_design_edit.cc:682:execute$2.c ;
  (* src = "./Src/and2.v:11.9-11.12" *)
  wire [1:0] \$auto$rs_design_edit.cc:682:execute$2.clk ;
  (* src = "./Src/and2.v:74.8-74.20" *)
  wire \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$a ;
  (* src = "./Src/and2.v:78.8-78.20" *)
  wire \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$b ;
  (* init = 1'h0 *)
  (* src = "./Src/and2.v:84.8-84.20" *)
  wire \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$c ;
  (* src = "./Src/and2.v:89.8-89.24" *)
  wire \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$reset ;
  (* src = "./Src/and2.v:73.14-73.26" *)
  wire [1:0] \$auto$rs_design_edit.cc:682:execute$2.intr_wire499 ;
  (* src = "./Src/and2.v:12.9-12.14" *)
  wire \$auto$rs_design_edit.cc:682:execute$2.reset ;
  (* src = "./Src/and2.v:10.9-10.10" *)
  wire \$auto$rs_design_edit.cc:682:execute$2.b ;
  (* src = "./Src/and2.v:53.14-53.20" *)
  wire [1:0] \$auto$rs_design_edit.cc:682:execute$2.io_clk ;
  (* src = "./Src/and2.v:8.10-8.13" *)
  (* src = "./Src/and2.v:8.10-8.13" *)
  wire out;
  (* src = "./Src/and2.v:12.9-12.14" *)
  (* src = "./Src/and2.v:12.9-12.14" *)
  wire reset;
  (* keep = 32'd1 *)
  (* src = "./Src/and2.v:13.14-13.15" *)
  (* keep = 32'd1 *)
  (* src = "./Src/and2.v:13.14-13.15" *)
  wire c;
  (* src = "./Src/and2.v:73.14-73.26" *)
  wire [1:0] intr_wire499;
  (* src = "./Src/and2.v:89.8-89.24" *)
  wire \$iopadmap$reset ;
  (* init = 1'h0 *)
  (* src = "./Src/and2.v:84.8-84.20" *)
  wire \$iopadmap$c ;
  (* src = "./Src/and2.v:78.8-78.20" *)
  wire \$iopadmap$b ;
  (* src = "./Src/and2.v:74.8-74.20" *)
  wire \$iopadmap$a ;
  (* src = "./Src/and2.v:10.9-10.10" *)
  (* src = "./Src/and2.v:10.9-10.10" *)
  wire b;
  (* src = "./Src/and2.v:9.9-9.10" *)
  (* src = "./Src/and2.v:9.9-9.10" *)
  wire a;
  (* src = "./Src/and2.v:11.9-11.12" *)
  (* src = "./Src/and2.v:11.9-11.12" *)
  wire [1:0] clk;
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/pp/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$and2.a  (
    .EN(1'h1),
    .I(\$auto$rs_design_edit.cc:682:execute$2.a ),
    .O(\$auto$rs_design_edit.cc:682:execute$2.$iopadmap$a )
  );
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/pp/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$and2.b  (
    .EN(1'h1),
    .I(\$auto$rs_design_edit.cc:682:execute$2.b ),
    .O(\$auto$rs_design_edit.cc:682:execute$2.$iopadmap$b )
  );
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/pp/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:41.13-41.44" *)
  O_BUF \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$and2.c  (
    .I(\$auto$rs_design_edit.cc:682:execute$2.$iopadmap$c ),
    .O(\$auto$rs_design_edit.cc:682:execute$2.c )
  );
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/pp/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$and2.clk  (
    .EN(1'h1),
    .I(\$auto$rs_design_edit.cc:682:execute$2.clk [0]),
    .O(\$auto$rs_design_edit.cc:682:execute$2.io_clk [0])
  );
  (* module_not_derived = 32'd1 *)
  (* src = "./Src/and2.v:133.5-137.4" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$and2.clk1  (
    .EN(1'h1),
    .I(\$auto$rs_design_edit.cc:682:execute$2.clk [1]),
    .O(\$auto$rs_design_edit.cc:682:execute$2.io_clk [1])
  );
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/pp/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$and2.reset  (
    .EN(1'h1),
    .I(\$auto$rs_design_edit.cc:682:execute$2.reset ),
    .O(\$auto$rs_design_edit.cc:682:execute$2.$iopadmap$reset )
  );
  (* module_not_derived = 32'd1 *)
  (* src = "/nfs_scratch/scratch/eda/behzad/pp/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:14.13-14.45" *)
  CLK_BUF \$auto$rs_design_edit.cc:682:execute$2.$auto$clkbufmap.cc:261:execute$497  (
    .I(\$auto$rs_design_edit.cc:682:execute$2.io_clk [0]),
    .O(\$auto$rs_design_edit.cc:682:execute$2.intr_wire499 [0])
  );
  (* module_not_derived = 32'd1 *)
  (* src = "./Src/and2.v:151.11-154.4" *)
  CLK_BUF \$auto$rs_design_edit.cc:682:execute$2.$auto$clkbufmap.cc:261:execute$4971  (
    .I(\$auto$rs_design_edit.cc:682:execute$2.io_clk [1]),
    .O(\$auto$rs_design_edit.cc:682:execute$2.intr_wire499 [1])
  );
  fabric_and2 \$auto$rs_design_edit.cc:680:execute$1  (
    .\$iopadmap$a (\$iopadmap$a ),
    .\$iopadmap$b (\$iopadmap$b ),
    .\$iopadmap$c (\$iopadmap$c ),
    .\$iopadmap$reset (\$iopadmap$reset ),
    .intr_wire499(intr_wire499),
    .out(out)
  );
  assign \$iopadmap$a  = \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$a ;
  assign \$iopadmap$b  = \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$b ;
  assign \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$c  = \$iopadmap$c ;
  assign \$iopadmap$reset  = \$auto$rs_design_edit.cc:682:execute$2.$iopadmap$reset ;
  assign \$auto$rs_design_edit.cc:682:execute$2.a  = a;
  assign \$auto$rs_design_edit.cc:682:execute$2.b  = b;
  assign c = \$auto$rs_design_edit.cc:682:execute$2.c ;
  assign \$auto$rs_design_edit.cc:682:execute$2.clk  = clk;
  assign intr_wire499 = \$auto$rs_design_edit.cc:682:execute$2.intr_wire499 ;
  assign \$auto$rs_design_edit.cc:682:execute$2.reset  = reset;
endmodule
