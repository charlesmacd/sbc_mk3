`include "globals.inc"
`include "timebase.inc"
/*
	Memory decoding:
	
	000000-0FFFFF : Flash
	100000-1FFFFF : Work RAM
	200000-2FFFFF : EEPROM
	300000-3FFFFF : Expansion CE# (2x32-pin header)
	400000-DFFFFF : (Unallocated, unused)
	E00000-EFFFFF : Flash (Mirror)
	F00000-FFFFFF : PIO (when A15=0); Unallocated, unused (when A15=1)
*/

module memory_decoder(
	
	/* CPU interface */
	input wire as_n,
	input wire [23:20] adh,
	input wire [15:12] adm,
	
	/* Chip enables */
	output wire flash_ce,
	output wire wram_ce,
	output wire eeprom_ce,
	output wire expansion_ce,
	output wire pio_ce,
	input wire overlay_n
	);
	
	wire [15:0] segment;
	
	/* Generate 16 enables for each megabyte of the address space */
	decoder #(16) segment_decoder (
		.select(adh[23:20]),
		.enable(~as_n),
		.y(segment)
		);
	
	/* 000000-0FFFFF : Flash CE# */
	/* E00000-EFFFFF : Flash CE# */
	assign flash_ce = ~(segment[4'h0]| segment[4'hE]);
	
	/* 100000-1FFFFF : WRAM CE# */
	assign wram_ce  = ~(segment[4'h1]);
	
	/* 200000-2FFFFF : EEPROM CE# */
	assign eeprom_ce = ~(segment[4'h2]);
	
	/* 300000-3FFFFF : Expansion enable */
	assign expansion_ce = ~(segment[4'h3]);
	
	/* FF8000-FFFFFF : PIO enable */
	assign pio_ce = (segment[4'hF] & adm[15]);
			
endmodule /* memory_decoder */
