/* Generated by Yosys 0.18+10 (git sha1 8ecc445e4, gcc 11.4.0-1ubuntu1~22.04 -fPIC -Os) */

module ft(din, dout);
  input din;
  output dout;
  wire \$flatten$auto$rs_design_edit.cc:682:execute$395.$iopadmap$din ;
  wire \$flatten$auto$rs_design_edit.cc:682:execute$395.$iopadmap$dout ;
  (* keep = 32'd1 *)
  (* src = "ft.v:11.10-11.14" *)
  wire \$auto$rs_design_edit.cc:682:execute$395.dout ;
  (* keep = 32'd1 *)
  (* src = "ft.v:10.9-10.12" *)
  wire \$auto$rs_design_edit.cc:682:execute$395.din ;
  (* keep = 32'd1 *)
  (* src = "ft.v:10.9-10.12" *)
  (* keep = 32'd1 *)
  (* src = "ft.v:10.9-10.12" *)
  wire din;
  (* keep = 32'd1 *)
  (* src = "ft.v:11.10-11.14" *)
  (* keep = 32'd1 *)
  (* src = "ft.v:11.10-11.14" *)
  wire dout;
  wire \$iopadmap$dout ;
  wire \$iopadmap$din ;
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/home/cschai/github/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:41.13-41.44" *)
  O_BUF \$flatten$auto$rs_design_edit.cc:682:execute$395.$iopadmap$ft.dout  (
    .I(\$flatten$auto$rs_design_edit.cc:682:execute$395.$iopadmap$dout ),
    .O(\$auto$rs_design_edit.cc:682:execute$395.dout )
  );
  (* keep = 32'd1 *)
  (* module_not_derived = 32'd1 *)
  (* src = "/home/cschai/github/yosys_verific_rs/yosys/install/bin/../share/yosys/rapidsilicon/genesis3/io_cell_final_map.v:29.41-29.81" *)
  I_BUF #(
    .WEAK_KEEPER("NONE")
  ) \$flatten$auto$rs_design_edit.cc:682:execute$395.$iopadmap$ft.din  (
    .EN(1'h1),
    .I(\$auto$rs_design_edit.cc:682:execute$395.din ),
    .O(\$flatten$auto$rs_design_edit.cc:682:execute$395.$iopadmap$din )
  );
  fabric_ft \$auto$rs_design_edit.cc:680:execute$394  (
    .\$iopadmap$din (\$iopadmap$din ),
    .\$iopadmap$dout (\$iopadmap$dout )
  );
  assign \$iopadmap$din  = \$flatten$auto$rs_design_edit.cc:682:execute$395.$iopadmap$din ;
  assign \$flatten$auto$rs_design_edit.cc:682:execute$395.$iopadmap$dout  = \$iopadmap$dout ;
  assign \$auto$rs_design_edit.cc:682:execute$395.din  = din;
  assign dout = \$auto$rs_design_edit.cc:682:execute$395.dout ;
endmodule
