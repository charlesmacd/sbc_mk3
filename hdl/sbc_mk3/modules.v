`include "globals.inc"
`include "timebase.inc"

module strobe_decode(
	input wire as_n,
	input wire lds_n,
	input wire uds_n,
	input wire rw_n,
	output wire lwr_n,
	output wire uwr_n,
	output wire rd_n
	);
	
	assign lwr_n = ~(!as_n & !lds_n & !rw_n);
	assign uwr_n = ~(!as_n & !uds_n & !rw_n);

	/* NOTE: This extra gating is needed for the SCC2961 UART which does not like to see RD# asserted
	         before the address has settled. The gating ensures the RD# is only pulsed when both the
				address and data strobes are asserted.
	*/
	assign rd_n  = ~((!as_n & !lds_n & rw_n) | (!as_n & !uds_n & rw_n));
	
endmodule /* strobe_decode */



//---------------------------------------------------------------------
//---------------------------------------------------------------------

	
//---------------------------------------------------------------------
//---------------------------------------------------------------------


module reset_driver(input wire reset_n, inout wire xreset_n, inout wire xhalt_n);

OPNDRN od_reset (.in(reset_n), .out(xreset_n));
OPNDRN od_halt  (.in(reset_n), .out(xhalt_n));

//assign xreset_n = ~reset_n ? 1'b0 : 1'bz;
//assign xhalt_n  = ~reset_n ? 1'b0 : 1'bz;		

endmodule /* reset_driver */

module pio_read_decoder(
	input wire enable,
	input wire rw_n,
	input wire [14:12] address,
	output wire [7:0] pio_output_enable
	);
	
	wire read_enable;
	assign read_enable = enable & rw_n;
	
	decoder #(8) segment_decoder (
		.select(address),
		.enable(enable),
		.y(pio_output_enable)
		);
	
endmodule /* pio_read_decoder */


module parallel_synchronizer #(parameter width = 8) (
	input wire clock,
	input wire reset_n,
	input wire [width-1:0] a,
	output wire [width-1:0] y
	);
	
	genvar i;
	generate 
	for(i = 0; i < width; i = i + 1)
	begin : synchronizers
		synchronizer PITSYNC (
			.clock(clock), 
			.reset_n(reset_n), 
			.a(a[i]), 
			.y(y[i])
			);
	end
	endgenerate
		
endmodule /* timer_edge_detector */


module parallel_edge_detector #(parameter width = 8) (
	input wire clock,
	input wire reset_n,
	input wire [width-1:0] channel_in,
	output wire [width-1:0] channel_out_pe,
	output wire [width-1:0] channel_out_ne
	);
	
	genvar i;
	generate 
	for(i = 0; i < width; i = i + 1)
	begin : detectors
		edge_detector PITEDGE (
			.clock(clock), 
			.reset_n(reset_n), 
			.a(channel_in[i]), 
			.ne(channel_out_ne[i]), 
			.pe(channel_out_pe[i])
			);
	end
	endgenerate
		
endmodule /* timer_edge_detector */
	
