`include "globals.inc"
`include "timebase.inc"


// other funcs
// ram overlay (swap out flash for ram)
// lfsr?
// status flag for delays? (say 1ms time?)
//
//
//
// flash checker
// have mirror of flash at a high memory area
// on low access force berr
// have enable bit
// bit=0
// 	term cycles in 0mb with dtack
// bit=1
//  	term cycles in 1mb with berr?
//		but need to tie berr to spare pin to use
// 	else could force nmi?
	
module sbc_glue(
	input wire cpuclock,
	input wire x8m,
	input wire sysrst_n,
	
	/* 68000 interface */
	inout wire xreset_n,
	inout wire xhalt_n,
	input wire xas_n,
	input wire xlds_n,
	input wire xuds_n,
	input wire xrw_n,
	input wire [2:0] xfc,
	inout wire xdtack_n,
	inout wire [2:0] xipl_n,
	output wire xvpa_n,	
	inout wire [7:0] xdb,
	input wire [23:20] xadh,
	input wire [15:12] xadm,
	input wire [7:6] xadl,

	
	/* Memory strobes */
	output wire mlwr_n,
	output wire muwr_n,
	output wire mrd_n,
	
	/* Local memory */
	output wire flash_ce_n,
	output wire eeprom_ce_n,
	output wire wram_ce_n,
	output wire expansion_cs_n,
	
	/* UM245R interface */
	output wire usb_wr,
	output wire usb_rd_n,
	input wire usb_txe_n,
	input wire usb_rxf_n,
	input wire usb_pwe_n,
	
	/* PIT interface */
	input wire systick_1ms,
	input wire interval_1ms,
	input wire interval_1us,
	
	/* UART interface */
	output wire uart_cs_n,
	input wire uart_int_n,
	
	/* Switches */
	input wire psw_break_n,
	
	/* Extra I/Os */
	output wire pcf_cs_n,
	input wire pcf_int_n,
	output wire pcf_iack_n,
		
	/* SPI interface */
	input wire spi_miso,
	output wire spi_mosi,
	output wire spi_sclk,

	/* Peripherals */
	output wire pit_cs_n,
	output wire post_ld_n,
	output wire outp_ld_n,
	output wire pio2_ld_n,
	output wire spare
	
	);


	wire [7:0] status_flags;
	wire [7:0] pio_page_oe;
	
	wire intc_ce;
	wire pio_page_oe_en;
	wire flash_ce;
	wire wram_ce;
	wire eeprom_ce;
	wire expansion_cs;
	wire pio_cs;	
	
	wire usb_write_wait;
	wire usb_read_wait;
	wire dtack_wait_terms;
	wire txe_s;
	wire rxf_s;	
	wire timer0_ne;
	wire timer1_ne;
	wire timer2_ne;
	wire overlay_we;
	wire overlay_n;
	wire psw_debounced;
	wire psw_debounced_ne;
	wire spi_ce;

	/* Split PIO area into eight pages 
	   FF8000-FF8FFF : Y0
	   FF9000-FF9FFF : Y1
	   FFA000-FFAFFF : Y2
	   FFB000-FFBFFF : Y3
	   FFC000-FFCFFF : Y4
	   FFD000-FFDFFF : Y5
	   FFE000-FFEFFF : Y6
	   FFF000-FFFFFF : Y7
	*/
   /* Read strobe enable for sub decode */
	assign pio_page_oe_en = pio_cs & xrw_n;	

	decoder #(8) PIO_PAGE_DEC (
		.select(xadm[14:12]),
		.enable(pio_page_oe_en),
		.y(pio_page_oe)
		);
		
		
	wire xmlwr_n;
	wire xmuwr_n;

	/* 68000 and memory strobes */
	strobe_decode HOST_STROBE_DEC (
		.as_n(xas_n),
		.lds_n(xlds_n),
		.uds_n(xuds_n),
		.rw_n(xrw_n),
		.lwr_n(xmlwr_n),
		.uwr_n(xmuwr_n),
		.rd_n(mrd_n)
		);
		

	
	wire status_spi_dmago;
		
	// flash_ce_n
	// 
	assign mlwr_n = ~((~flash_ce_n & ~xmlwr_n & overlay_reg[3]) || (flash_ce_n & ~xmlwr_n));
	assign muwr_n = ~((~flash_ce_n & ~xmuwr_n & overlay_reg[3]) || (flash_ce_n & ~xmuwr_n));

	/* Memory decoder and Flash/WRAM overlay */	
	memory_decoder TOP_MEMDEC (
		.as_n(xas_n),
		.adh(xadh),
		.adm(xadm),
		.flash_ce(flash_ce_n),
		.wram_ce(wram_ce_n),
		.eeprom_ce(eeprom_ce_n),
		.expansion_ce(expansion_cs_n),
		.pio_ce(pio_cs),
		.overlay_n(overlay_n)
	);
	
	wire top_memdec_dtack;
	assign top_memdec_dtack = !flash_ce_n | !wram_ce_n | !eeprom_ce_n | !expansion_cs_n | pio_cs;
	
	/* PIO area decode */
	pio_write_strobe_decode PIO_WS_DEC(
		.cs(pio_cs),
		.rw_n(xrw_n),
		.adm(xadm[14:12]),
		.adl(xadl[7:6]),
		.post_ld_n(post_ld_n),
		.outp_ld_n(outp_ld_n),
		.pio2_ld_n(pio2_ld_n),
		.spi_ce(spi_ce),
		.uart_cs_n(uart_cs_n),
		.pit_cs_n(pit_cs_n),
		.pcf_cs_n(pcf_cs_n),
		.usb_wr(usb_wr),
		.usb_rd_n(usb_rd_n),
		.intc_ce(intc_ce),
		.overlay_we(overlay_we)
		);

	/* Edge detector to detect negative edge of PIT channel outputs
	   Used to convert output pulse (which can be 1us or 1ms long) to a 8 MHz pulse
		Do we want to use PE pulse instead? (validate timing)
	*/
	parallel_edge_detector #(3) PIT_OUT_ED (
		.clock(x8m),
		.reset_n(sysrst_n),
		.channel_in({interval_1ms, interval_1us, systick_1ms}),
		.channel_out_ne({timer2_ne, timer1_ne, timer0_ne})
		);
	
	/* Synchronizer on UM245R TXE,RXF outputs */
	parallel_synchronizer #(2) USBSYNCH (
		.clock(x8m),
		.reset_n(sysrst_n),
		.a({usb_txe_n, usb_rxf_n}),
		.y({txe_s, rxf_s})
	);
	
	/* CPU signals */
	reset_driver RST_DRV (.reset_n(sysrst_n), .xreset_n(xreset_n), .xhalt_n(xhalt_n));

	/* Detect BREAK button with 1us resolution */
	edge_detector BRK_EDGE_1US (.clock(systick_1ms), .reset_n(sysrst_n), .a(psw_break_n),  .ne(psw_debounced));	/* 1us pulse width */ 
	
	/* Detect BREAK button with 125ns resolution */
	edge_detector BRK_EDGE_1MS (.clock(x8m), .reset_n(sysrst_n), .a(psw_debounced),  .pe(psw_debounced_ne));	/* 1us pulse width */ 

	/* Overlay register */
	
	/*
		OVERLAY_REG
		D3 : FLASH_WE
		D2 : SPARE bit
		D1 : SPI SCLK
		D0 : SPI MOSI
	*/
	wire [7:0] overlay_reg;
	flopren #(8) OVERLAY_REG4B (
		.clock(x8m), 
		.reset_n(sysrst_n), 
		.enable(overlay_we), 
		.d(xdb[7:0]), 
		.q(overlay_reg)
		);
//	assign spi_mosi = overlay_reg[0];
	assign spi_sclk = spi_clock;
	assign spare = overlay_reg[2];
	assign pio_spi_clock = overlay_reg[0];
		
	
	/* Status word */
	assign status_flags[0] = psw_break_n;
	assign status_flags[1] = spi_miso;
	assign status_flags[2] = status_spi_dmago;
	assign status_flags[3] = uart_int_n;
	assign status_flags[4] = pcf_int_n;
	assign status_flags[5] = usb_pwe_n;
	assign status_flags[6] = usb_txe_n;
	assign status_flags[7] = usb_rxf_n;
	
	/******************************************************/
	/******************************************************/
	/******************************************************/
	/******************************************************/
	/******************************************************/
	/******************************************************/

	/* Insert wait states when writing to a full TX FIFO */
	assign usb_write_wait = sysrst_n &  usb_wr   & txe_s;
	
	/* Insert wait states when reading from an empty RX FIFO */
	assign usb_read_wait  = sysrst_n & ~usb_rd_n & rxf_s;
	
	/* Combine all wait terms together */
	assign dtack_wait_terms = (usb_write_wait | usb_read_wait);
	
	/* Detect interrupt acknowledge cycle */
	wire iack_cycle;
	assign iack_cycle = !xas_n & xfc[2] & xfc[1] & xfc[0];
		
	/* Terminate interrupt acknowledge cycles with VPA# to initiate autovectored interrupt */
	assign xvpa_n = ~iack_cycle;
	
	/* Terminate bus cycles at full speed unless wait terms exist */
	
	// top_memdec_dtack is 1 when accessing mem/pio
	assign xdtack_n = (dtack_wait_terms ^ top_memdec_dtack) ? 1'b0 : 1'bz;	
	
	// TOP DWT xdout
	//  0   0  1'bz			not accessing mem/pio,no wait terms, must be external
	//  1   0  1'b0         accessing memory, no wait terms, term cycle immediately
	//  0   1  1'b0         not accessing mem/pio, wait terms (how can this happen?)
	//  1   1  1'bz         accessing memory, wait terms, wait
	
	// drive dtack low normally, but tristate during wait terms
	// instead
	// normally tristate
	// if wait terms tristate
	
	/*============================================================*/
	// Databus driver section
	// cpld drives out data bus for
	// ff8000-ff8fff
	// ffb000-ffbfff
	/*============================================================*/
	
	wire dbus_oe;
	wire [7:0] databus_int;
	assign dbus_oe = pio_page_oe[3] | pio_page_oe[0];
	
	wire [7:0] status_int;
	wire [7:0] spi_q;
	assign spi_q = 8'hA5;
	
	mux4 #(8) STATUS_MUX (
		.select(xadl[7:6]),
		.a0(8'h81),
		.a1(status_flags),
		.a2(spi_clocks[5:0]),
		.a3(spi_q),
		.y(status_int)
		);

	mux8 #(8) DBUS_MUX8 (
		.select(xadm[14:12]),
		.a0(status_int),
		.a3(intc_databus_out),
		.y(databus_int)
		);
		
	assign xdb = (dbus_oe) ? databus_int : 8'bzzzzzzzz;

	wire [7:0] intc_databus_out;
	
	/*============================================================*/
	/* Unused signals */
	/*============================================================*/

	assign pcf_iack_n = 1'b1; /* Disable vector output from PCF */

	/*============================================================*/
	/* Interrupt controller */
	/*============================================================*/
	
	// xadl7,6 for decoding
	// intc_ce and xrwn to generate four we strobes
	// x8m and sysrstn
   // xdb in/out
	// xipl
   //	inputs: .inputs({psw_debounced_ne, timer0_ne, timer1_ne, timer2_ne}),
	
	// ffb0x0
	
	new_interrupt_controller NEW_INTCON (
		.clock(x8m),
		.reset_n(sysrst_n),
		.chip_enable(intc_ce),
		.read_write_n(xrw_n),
		.host_din(xdb),
		.host_qout(intc_databus_out),
		.host_address(xadl[7:6]),
		.host_ipl_n(xipl_n),
//		.irq({1'b0, 1'b0, 1'b0, uart_int_n, psw_debounced_ne, timer0_ne, timer1_ne, timer2_ne})
		.irq({1'b0, 1'b0, 1'b0, uart_int_n, psw_debounced_ne, timer0_ne, timer1_ne, timer2_ne})
		);

		//=====================================
		
	wire pio_spi_clock;
	wire spi_clock;
	wire [5:0] spi_clocks;
	/***************************
	-- this works!
	-- used to clock bin counter once after each read cycle
	- or is it before?
	wire strobe;
	wire strobe_edge;
	// read from 
	// 1111 1xxx xxxx xxxx xxxx
	// ---- 1000 ---- 10-- ---- // f8080
	assign strobe = !xas_n & xrw_n & pio_cs & (xadl[7:6] == 2'b10) && (xadm[15:12] == 4'b1000);
	
	edge_detector U1X0 (
		.clock(x8m),
		.reset_n(sysrst_n),
		.a(strobe),
		.ne(strobe_edge),
		);
	**********************************/
	binary_counter #(6) SPI_CLKGEN (
		.clock(x8m),
		.reset_n(sysrst_n),
		.q(spi_clocks)
		);
		
	wire spi_clock_int;
		
	mux8 #(1) SPI_CLKMUX (
		.select(overlay_reg[7:5]),
		.a0(x8m), /* 8 MHz */
		.a1(spi_clocks[0]), /* 4 MHz */
		.a2(spi_clocks[1]), /* 2 MHz */
		.a3(spi_clocks[2]), /* 1 MHz */
		.a4(spi_clocks[3]), /* 500 KHz */
		.a5(spi_clocks[4]), /* 250 KHz */
		.a6(spi_clocks[5]), /* 125 KHz */
		.a7(pio_spi_clock), /* Manual */
		.y(spi_clock_int)
		);
		
	assign spi_clock = spi_clock_int ^ overlay_reg[4];
		
	wire spi_write;
	wire spi_read;
	wire spi_write_ne;
	
	// spi_ce has as already
	assign spi_write = spi_ce & !xrw_n;
	assign spi_read  = spi_ce &  xrw_n;
	
	edge_detector(
		.clock(x8m),
		.reset_n(sysrst_n),
		.a(spi_write),
		.ne(spi_write_ne)
		);
		

	// data is loaded by pre/clr
	// shifting is by clk,
	// 
	
	// Data is loaded in parallel by asserting LOAD_N,
	// which internally asserts each PREn CLRn
	// this ignores D, CLK, ENA and has the most priority
	// 
	
	wire spi_miso_sampled;
	wire spi_selected_clock;
	wire spi_dmago_clear_n;
	
	// latch miso on opposite edge
	DFFE SPI_MISO_SAMPLE (
		.d(spi_miso),
		.clk(~spi_clock),
		.clrn(1'b1),
		.prn(1'b1),
		.ena(1'b1),
		.q(spi_miso_sampled)
	);
		
		
	DFFE DFFE_DMAGO (
		.d(1'b1),
		.clk(1'b1),
		.clrn(spi_dmago_clear_n),
		.prn(spi_write_ne),
		.ena(1'b0),
		.q(status_spi_dmago)
	);
		
	shift_register_left #(8) SPI_SREG (
		.clock(spi_clock),
		.enable(status_spi_dmago),
		.load_n(~spi_write),
		.din(xdb[7:0]),
		.serial_in(spi_miso_sampled),
		.serial_out(spi_mosi)
		);

reg [3:0] spi_cnt;
wire spi_cnt_tc;

always@(posedge spi_clock, negedge sysrst_n)
begin
	if(!sysrst_n)
	begin
		spi_cnt <= 4'h0;
	end
	else
	begin
		if(spi_write_ne)
			spi_cnt <= 4'h0;
		else
		if(status_spi_dmago)
			spi_cnt <= spi_cnt + 4'h1;
	end
end

assign spi_cnt_tc = spi_cnt[3]; // range of 8-f


wire temp;
	edge_detector(
		.clock(x8m),
		.reset_n(sysrst_n),
		.a(spi_cnt_tc),
		.ne(temp),
		);
assign spi_dmago_clear_n = ~temp;		


endmodule /* sbc_glue */	









/* End */
