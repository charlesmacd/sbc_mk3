`include "globals.inc"
`include "timebase.inc"
	
module pio_write_strobe_decode(

	/* Host interface */
	input wire cs,
	input wire rw_n,
	input wire [14:12] adm,
	input wire [7:6] adl,
	
	/* Chip enables */
	output wire post_ld_n,
	output wire outp_ld_n,
	output wire pio2_ld_n,
	output wire spi_ce,
	output wire uart_cs_n,
	output wire pit_cs_n,
	output wire pcf_cs_n,
	output wire usb_wr,
	output wire usb_rd_n,
	output wire intc_ce,
	output wire overlay_we,
	output wire debug_ce
	);
	
wire [7:0] enable_hi;
wire [3:0] enable_lo;
	
decoder #(8) main_dec (
	.select(adm[14:12]),
	.enable(cs),
	.y(enable_hi)
	);
decoder #(4) sub_dec (
	.select(adl[7:6]),
	.enable(cs),
	.y(enable_lo)
	);
	
/* FF80x0 */
assign post_ld_n   		= ~(enable_hi[0] & enable_lo[0] & !rw_n); // FF8000 W/O
assign outp_ld_n   		= ~(enable_hi[0] & enable_lo[1] & !rw_n); // FF8040 W/O
assign pio2_ld_n   		= ~(enable_hi[0] & enable_lo[2] & !rw_n); // FF8080 W/O
assign spi_ce           =  (enable_hi[0] & enable_lo[3]        ); // FF80C0 W/O

/* FF90x0 */
assign uart_cs_n   		= ~(enable_hi[1] & enable_lo[0]);			// FF9000 R/W
assign pit_cs_n    		= ~(enable_hi[1] & enable_lo[1]);			// FF9040 R/W
assign pcf_cs_n    		= ~(enable_hi[1] & enable_lo[2] & !rw_n);	// FF9080 W/O
assign overlay_we		   =  (enable_hi[1] & enable_lo[3] & !rw_n);	// FF90C0 W/O

/* FFA0x0 */
assign usb_wr      		=  (enable_hi[2] & enable_lo[0] & !rw_n);		// FFA000 W/O 
assign usb_rd_n    		= ~(enable_hi[2] & enable_lo[0] &  rw_n);		// FFA000 R/O

assign debug_ce         =  enable_hi[2] & enable_lo[2]  // ffa080 r/w
                        |  enable_hi[2] & enable_lo[3]; // ffa0c0 r/w

/* FFB0x0 */
assign intc_ce				=	enable_hi[3];									// FFB0x0 R/W

endmodule /* pio_decode */

	