`default_nettype none


module loadable_counter #(parameter width = 8) (
	input wire clock,
	input wire reset_n,
	input wire load_async_n,
	input wire count_enable,
	input wire [width-1:0] d,
	output wire [width-1:0] q
	);
	
	wire [width-1:0] clrn;
	wire [width-1:0] prn;
	wire [width-1:0] source;
	
	assign source = count_enable ? (q + 'b1) : q;
	
	genvar i;
	generate
	for(i = 0; i < width; i = i + 1)
	begin : ldcnt
	
		assign clrn[i] = ~(!load_async_n & !d[i]) | !reset_n;
		assign prn[i]  = ~(!load_async_n &  d[i]);
	
		DFFE ff (.clk(clock), .d(source[i]), .q(q[i]), .clrn(clrn[i]), .prn(prn[i]), .ena(count_enable));
	end
	endgenerate
	
endmodule /* host_register */



module host_register #(parameter width = 8) (
	input wire clock,
	input wire reset_n,
	input wire load_enable,
	input wire [width-1:0] d,
	output wire [width-1:0] q
	);
	
	genvar i;
	generate
	for(i = 0; i < width; i = i + 1)
	begin : storage
		DFFE ff (.clk(clock), .d(d[i]), .q(q[i]), .clrn(reset_n), .prn(1'b1), .ena(load_enable));
	end
	endgenerate
	
endmodule /* host_register */



/* Register file */
module host_register_block #(parameter count = 4, width = 8) (
	input wire host_cs_n,
	input wire host_wr_n,
	input wire reset_n,
	input wire [$clog2(count-1):0] host_address,
	input wire [width-1:0] host_data,
	output wire [count-1:0] external_enables,
	output wire [(count * width)-1:0] qout
	);

	wire [count-1:0] enables;
	wire enable;
	
	assign enable = ~host_cs_n;
	
	decoder #(count) host_write_decoder (
		.select(host_address),
		.enable(enable),
		.y(enables)
		);
	
genvar i;

wire wren;
assign wren = ~(!host_cs_n & !host_wr_n);

generate 
for(i = 0; i < count; i = i + 1)
begin : reg_block

	host_register #(width) reg_ele (
		.clock(wren),
		.load_enable(enables[i]),
		.reset_n(reset_n),
		.d(host_data),
		.q(qout[i * width +: width])
		);
end
endgenerate	

assign external_enables = enables;
	
endmodule /* host_register_block */
