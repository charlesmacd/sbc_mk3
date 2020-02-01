/*
	change cpld pinout
	to put global oe on prd_n

			read					write
	0:								adl	
	1:								adm
	2:								adh
	3:		hudata+hrd_n		hudata+hrd_n
	4:    						hudata_rx
	5:		cd,irq2           cd,irq2 rx, reset, hsm
	6:
	7:
	
	have special ports for read and auto-increment, and write-and-auto increment
	(can't have pe detector without clock)
*/



`default_nettype none

`define REG_ADL		3'b000
`define REG_ADM		3'b001
`define REG_ADH		3'b010
`define REG_DATA		3'b011
`define REG_DBRX		3'b100
`define REG_MISC		3'b101
`define REG_DEBUG		3'b110
`define REG_ID			3'b111
`define REG_ID_VALUE	8'hA5

/* 8 to 1 mux */
module mux8 #(parameter width = 8) (
	input wire [2:0] select,
	input wire [width-1:0] a0,
	input wire [width-1:0] a1,
	input wire [width-1:0] a2,
	input wire [width-1:0] a3,
	input wire [width-1:0] a4,
	input wire [width-1:0] a5,
	input wire [width-1:0] a6,
	input wire [width-1:0] a7,
	output reg [width-1:0] y
	);
	
	always@(*)
	begin
		case(select)	
		3'd0: y <= a0;
		3'd1: y <= a1;
		3'd2: y <= a2;
		3'd3: y <= a3;
		3'd4: y <= a4;
		3'd5: y <= a5;
		3'd6: y <= a6;
	default: y <= a7;
		endcase
	end
		
endmodule /* mux8 */



/* 1 to N decoder */
module decoder #(parameter width = 4)(
	input wire [$clog2(width)-1:0] select,
	input wire enable,
	output wire [width-1:0] y
	);
	
	assign y = enable ? (1'b1 << select) : {width{1'b0}};

endmodule /* decoder */




/* Register */
module flopren #(parameter width = 8)(
	input wire clock,
	input wire reset_n,
	input wire enable,
	input wire [width-1:0] d,
	output reg [width-1:0] q
	);
	
	always@(posedge clock, negedge reset_n)
	begin
		if(~reset_n)
			q <= {width{1'b0}};
		else
			q <= (enable) ? d : q;
	end	
	
endmodule /* flopren */


module field_reverse_unidirectional #(parameter width = 8) (
	input [width-1:0] a,
	output [width-1:0] y
	);
	
genvar i;
generate
	for(i = 0; i < width; i = i+1)
	begin : map
		assign y[i] = a[width-i];
	end
endgenerate
	
endmodule /* field_reverse */


/* Strobe generator */
module strobe_generator (
	input wire host_cs_n,
	input wire host_wr_n,
	input wire host_rd_n,
	input wire [3:1] host_address,
	output wire card_wr_n,
	output wire card_rd_n,
	output wire rx_ld_n,
	output wire card_db_oe,
	output wire host_db_oe,
	output wire [7:0] host_reg_strobe
	);

	decoder #(8) PIO_DECODER (
		.select(host_address[3:1]),
		.enable(host_cs_n),
		.y(host_reg_strobe)
		);
	
	assign card_db_oe  =  ( host_reg_strobe[`REG_DATA] & !host_wr_n);
	assign card_wr_n   = ~( host_reg_strobe[`REG_DATA] & !host_wr_n);
	assign card_rd_n   = ~( host_reg_strobe[`REG_DATA] & !host_rd_n);
	assign rx_ld_n     = ~( host_reg_strobe[`REG_DBRX] & !host_wr_n);
	assign host_db_oe  =  (!host_cs_n & !host_rd_n);
	
endmodule /* strobe generator */




module top (

	/* Host interface */
	input wire pio_rst_n,
	input wire pexp_cs_n,
	input wire prd_n,
	input wire pwr_n,
	input wire [3:1] pa,
	inout wire [7:0] pd,
	
	/* Card interface */
	output wire [20:0] ha,
	inout wire [7:0] hd,
	input wire [7:0] hrx,
	output wire hwr_n,
	output wire hrd_n,
	output wire hsm,
	output wire hreset_n,
	input wire card_detect_n,
	input wire hirq2_n,

	output wire card_detect_n_rx,
	output wire hirq2_n_rx,
	output wire rx_ld,
	output wire spio0,
	output wire spio1,
	input wire x8m
	);
	
	wire [63:0] qdata;
	wire [7:0] status_int;
	wire write_clock_n;
	wire [7:0] enable;
	
	/* Write strobe to latch host data */
	assign write_clock_n = ~(pexp_cs_n == 0 && pwr_n == 0);
	

	/* Decode registers */
	decoder #(8) (
		.select(pa[3:1]),
		.enable(1'b1),
		.y(enable)
		);
		
	flopren  #(8) (.clock(write_clock_n), .reset_n(pio_rst_n), .enable(enable[`REG_ADL]),  .d(pd), .q(qdata[(`REG_ADL  * 8) +: 8]) );
	flopren  #(8) (.clock(write_clock_n), .reset_n(pio_rst_n), .enable(enable[`REG_ADM]),  .d(pd), .q(qdata[(`REG_ADM  * 8) +: 8]) );
	flopren  #(8) (.clock(write_clock_n), .reset_n(pio_rst_n), .enable(enable[`REG_ADH]),  .d(pd), .q(qdata[(`REG_ADH  * 8) +: 8]) );
//	flopren  #(8) (.clock(write_clock_n), .reset_n(pio_rst_n), .enable(enable[`REG_DBRX]), .d(pd), .q(qdata[(`REG_DBRX * 8) +: 8]) );
	flopren  #(8) (.clock(write_clock_n), .reset_n(pio_rst_n), .enable(enable[`REG_MISC]), .d(pd), .q(qdata[(`REG_MISC * 8) +: 8]) );
	
	assign ha[7:0]   = debug_out;//qdata[(`REG_ADL * 8) +: 8];
	assign ha[15:8]  = qdata[(`REG_ADM * 8) +: 8];
	assign ha[20:16] = qdata[(`REG_ADH * 8) +: 8];
	
	assign {hreset_n, hsm, card_detect_n_rx, hirq2_n_rx} = qdata[(`REG_MISC * 8) +: 8];
	
	wire [7:0] misc_status;
	
	assign misc_status = {2'b00, card_detect_n, hirq2_n, hreset_n, hsm, card_detect_n_rx, hirq2_n_rx};
	
	mux8 #(8) (
		.select(pa[3:1]),
		.a0(qdata[(`REG_ADL  * 8) +: 8]),
		.a1(qdata[(`REG_ADM  * 8) +: 8]),
		.a2(qdata[(`REG_ADH  * 8) +: 8]),
		.a3(hd/*qdata[(`REG_DBRX * 8) +: 8]*/),
		.a4(hrx/*qdata[(`REG_MISC * 8) +: 8]*/),
		.a5(misc_status),
		.a6(debug_out),
		.a7(hd),
		.y(status_int)
	);
	
	/* Latch data */
	assign rx_ld = !pexp_cs_n & enable[`REG_DBRX] & !pwr_n;
	
	/* HuCard read/write strobes */
	
	wire term0;
	wire term1;
	
	assign term0 = (!pexp_cs_n & enable[`REG_DATA] & !prd_n);
	assign term1 = (!pexp_cs_n & enable[7]         & !prd_n);
	
	assign hrd_n = ~(term0 | term1);
					
	assign hwr_n = ~(!pexp_cs_n & enable[`REG_DATA] & !pwr_n);
		
	/* Drive host data onto HuCard data bus during a write cycle */
	assign hd = (!hwr_n) ? pd : 8'bzzzzzzzz;

	/* Drive host data during peripheral read */
	assign pd = (!pexp_cs_n & !prd_n) ? status_int : 8'bzzzzzzzz;

	wire [7:0] debug_out;
	wire read_clock;
	reg [7:0] cntr;

	wire cnt_en;
	wire cnt_ld_n;
	
	assign read_clock = !prd_n & !pexp_cs_n;
	assign cnt_en     = enable[7];
	assign cnt_ld_n   = ~(!pexp_cs_n & enable[`REG_ADL] & !pwr_n);

	assign spio0 = read_clock;
	assign spio1 = cnt_en;
		
	loadable_counter #(8) (
		.clock(read_clock),
		.reset_n(pio_rst_n),
		.load_async_n(cnt_ld_n), 
		.count_enable(cnt_en),
		.d(pd),
		.q(debug_out)
	);
	
	
endmodule






