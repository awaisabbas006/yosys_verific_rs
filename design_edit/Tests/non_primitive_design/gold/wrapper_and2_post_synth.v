/* Generated by Yosys 0.18+10 (git sha1 07c42e625, gcc 11.4.0-1ubuntu1~22.04 -fPIC -Os) */

module and2(a, b, c, clk, reset);
  output c;
  input b;
  input clk;
  input a;
  input reset;
  wire \$auto$clkbufmap.cc:294:execute$425 ;
  wire \$iopadmap$a ;
  wire \$iopadmap$clk ;
  wire \$auto$clkbufmap.cc:262:execute$424 ;
  wire \$iopadmap$reset ;
  wire \$iopadmap$c ;
  wire \$iopadmap$b ;
  (* keep = 32'd1 *)
  (* src = "./rtl/and2.v:18.8-18.9" *)
  (* keep = 32'd1 *)
  (* src = "./rtl/and2.v:18.8-18.9" *)
  wire c;
  (* src = "./rtl/and2.v:15.12-15.13" *)
  (* src = "./rtl/and2.v:15.12-15.13" *)
  wire b;
  (* src = "./rtl/and2.v:16.12-16.15" *)
  (* src = "./rtl/and2.v:16.12-16.15" *)
  wire clk;
  (* src = "./rtl/and2.v:14.12-14.13" *)
  (* src = "./rtl/and2.v:14.12-14.13" *)
  wire a;
  (* src = "./rtl/and2.v:17.12-17.17" *)
  (* src = "./rtl/and2.v:17.12-17.17" *)
  wire reset;
  fabric_and2 \$auto$rs_design_edit.cc:578:execute$434  (
    .\$iopadmap$clk (\$iopadmap$clk ),
    .\$auto$clkbufmap.cc:294:execute$425 (\$auto$clkbufmap.cc:294:execute$425 ),
    .\$iopadmap$a (\$iopadmap$a ),
    .\$iopadmap$b (\$iopadmap$b ),
    .\$iopadmap$c (\$iopadmap$c ),
    .\$iopadmap$reset (\$iopadmap$reset ),
    .\$auto$clkbufmap.cc:262:execute$424 (\$auto$clkbufmap.cc:262:execute$424 )
  );
  interface_and2 \$auto$rs_design_edit.cc:580:execute$435  (
    .\$iopadmap$clk (\$iopadmap$clk ),
    .clk(clk),
    .a(a),
    .b(b),
    .c(c),
    .reset(reset),
    .\$auto$clkbufmap.cc:294:execute$425 (\$auto$clkbufmap.cc:294:execute$425 ),
    .\$iopadmap$a (\$iopadmap$a ),
    .\$iopadmap$b (\$iopadmap$b ),
    .\$iopadmap$c (\$iopadmap$c ),
    .\$iopadmap$reset (\$iopadmap$reset ),
    .\$auto$clkbufmap.cc:262:execute$424 (\$auto$clkbufmap.cc:262:execute$424 )
  );
endmodule

module interface_and2(a, b, c, clk, reset, \$iopadmap$clk , \$iopadmap$b , \$iopadmap$c , \$iopadmap$a , \$iopadmap$reset , \$auto$clkbufmap.cc:294:execute$425 , \$auto$clkbufmap.cc:262:execute$424 );
  input reset;
  input a;
  output \$iopadmap$a ;
  output c;
  input b;
  input \$auto$clkbufmap.cc:262:execute$424 ;
  output \$iopadmap$reset ;
  input \$iopadmap$c ;
  output \$auto$clkbufmap.cc:294:execute$425 ;
  output \$iopadmap$b ;
  output \$iopadmap$clk ;
  input clk;
  (* src = "./rtl/and2.v:17.12-17.17" *)
  (* src = "./rtl/and2.v:17.12-17.17" *)
  wire reset;
  (* src = "./rtl/and2.v:14.12-14.13" *)
  (* src = "./rtl/and2.v:14.12-14.13" *)
  wire a;
  wire \$iopadmap$a ;
  (* keep = 32'd1 *)
  (* src = "./rtl/and2.v:18.8-18.9" *)
  (* keep = 32'd1 *)
  (* src = "./rtl/and2.v:18.8-18.9" *)
  wire c;
  (* src = "./rtl/and2.v:15.12-15.13" *)
  (* src = "./rtl/and2.v:15.12-15.13" *)
  wire b;
  wire \$auto$clkbufmap.cc:262:execute$424 ;
  wire \$iopadmap$reset ;
  wire \$iopadmap$c ;
  wire \$auto$clkbufmap.cc:294:execute$425 ;
  wire \$iopadmap$b ;
  wire \$iopadmap$clk ;
  (* src = "./rtl/and2.v:16.12-16.15" *)
  (* src = "./rtl/and2.v:16.12-16.15" *)
  wire clk;
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/home/cschai/github/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$iopadmap$and2.a  (
    .O(\$iopadmap$a ),
    .EN(1'h1),
    .I(a)
  );
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/home/cschai/github/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$iopadmap$and2.reset  (
    .O(\$iopadmap$reset ),
    .EN(1'h1),
    .I(reset)
  );
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/home/cschai/github/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:41.13-41.44" *)
  O_BUF \$iopadmap$and2.c  (
    .O(c),
    .I(\$iopadmap$c )
  );
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/home/cschai/github/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$iopadmap$and2.clk  (
    .O(\$iopadmap$clk ),
    .EN(1'h1),
    .I(clk)
  );
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/home/cschai/github/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$iopadmap$and2.b  (
    .O(\$iopadmap$b ),
    .EN(1'h1),
    .I(b)
  );
  (* module_not_derived = 32'd1 *)
  (* src = "/home/cschai/github/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:14.13-14.45" *)
  CLK_BUF \$auto$clkbufmap.cc:261:execute$423  (
    .O(\$auto$clkbufmap.cc:294:execute$425 ),
    .I(\$auto$clkbufmap.cc:262:execute$424 )
  );
endmodule