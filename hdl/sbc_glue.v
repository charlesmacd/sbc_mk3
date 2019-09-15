`default_nettype none




/*
					READ
	FF8000 :		STATUS LO		POST_LD
	FF8040 :		STATUS HI		OUTP_LD
	FF8080 :		STATUS LO		PIO2_LD
	FF80C0 :		STATUS HI		SPI_WRITE

	FF9000 :		UART				UART
	FF9040 :		PIT				PIT
	FF9080 :		PCF				PCF
	FF90C0 :		x					OVERLAY_WE
	
	FFA000 :		USB RD
	FFA040 :		USB RD
	FFA080 :		USB RD
	FFA0C0 :		USB RD

	FFB000 :		I PENDING
	FFB040 :		I MASKED (PEND & ENA
	FFB080 :		I ENABLE
	FFB0C0 :		IPL PRIORITY
	
*/

module mem_decoder(
	input wire as_n,
	input wire [23:20] adh,
	input wire [15:12] adm,
	output wire flash_ce,
	output wire wram_ce,
	output wire eeprom_ce,
	output wire expansion_ce,
	output wire pio_ce,
	input wire overlay_n
	);
	
	wire [15:0] ena;

	
	decoder #(16) mem_dec (
		.select(adh[23:20]),
		.enable(~as_n),
		.y(ena)
		);
	
	/* 000000-0FFFFF : Flash (OVL#=L) */
	/* E00000-EFFFFF : Flash (always) */
	assign flash_ce = ~(ena[0]| ena[14]);
	
	/* 100000-1FFFFF : WRAM (always) */
	/* 000000-1FFFFF : WRAM(OVL#=H)  */
	assign wram_ce  = ~(ena[1]);
	
	/* 200000-2FFFFF : EEPROM */
	assign eeprom_ce = ~(ena[2]);
	
	/* 300000-3FFFFF : Expansion */
	assign expansion_ce = ~(ena[3]);
	
	/* FF8000-FFFFFF : PIO */
	assign pio_ce = (ena[15] & adm[15]);
			
endmodule

module strobe_decode(
	input wire as_n,
	input wire lds_n,
	input wire uds_n,
	input wire rw_n,
	output wire lwr_n,
	output wire uwr_n,
	output wire rd_n,
	input wire [2:0] fc,
	output wire vpa_n
	);
	
	assign lwr_n = ~(!as_n & !lds_n & !rw_n);
	assign uwr_n = ~(!as_n & !uds_n & !rw_n);
	assign rd_n  = ~((!as_n & !lds_n & rw_n) | (!as_n & !uds_n & rw_n));
	assign vpa_n = ~(!as_n & fc[2] & fc[1] & fc[0]);
	
endmodule /* strobe_decode */



module pio_strobe_decode(
	input wire cs,
	input wire rw_n,
	input wire [14:12] adm,
	input wire [7:6] adl,
	
	output wire post_ld_n,
	output wire outp_ld_n,
	output wire pio2_ld_n,
	output wire spi_write,
	output wire uart_cs_n,
	output wire pit_cs_n,
	output wire pcf_cs_n,
	output wire usb_wr,
	output wire usb_rd_n,
	output wire intc_ce,
	output wire overlay_we
	);
	
wire [7:0] enah;
wire [3:0] enal;
	
	decoder #(8) main_dec (
		.select(adm[14:12]),
		.enable(cs),
		.y(enah)
		);
	decoder #(4) sub_dec (
		.select(adl[7:6]),
		.enable(cs),
		.y(enal)
		);
	
/* FF80x0 */
assign post_ld_n   		= ~(enah[0] & enal[0] & !rw_n);
assign outp_ld_n   		= ~(enah[0] & enal[1] & !rw_n);
assign pio2_ld_n   		= ~(enah[0] & enal[2] & !rw_n);
assign spi_write        = ~(enah[0] & enal[3] & !rw_n);

/* FF90x0 */
assign uart_cs_n   		= ~(enah[1] & enal[0]);
assign pit_cs_n    		= ~(enah[1] & enal[1]);
assign pcf_cs_n    		= ~(enah[1] & enal[2] & !rw_n);
assign overlay_we		   =  (enah[1] & enal[3] & !rw_n);

/* FFA0x0 */
assign usb_wr      		=  (enah[2] &           !rw_n);
assign usb_rd_n    		= ~(enah[2] &            rw_n);

/* FFB0x0 */
assign intc_ce				=	enah[3];

endmodule /* pio_decode */


//---------------------------------------------------------------------
//---------------------------------------------------------------------

module interrupt_controller #(parameter width = 8) (
	input wire clock,
	input wire reset_n,
	input wire rw_n,
	input wire cs,
	input wire [width-1:0] irq_n,
	input wire [width-1:0] data_in,
	output wire [width-1:0] data_out,
	input wire [1:0] offset,
	output wire [$clog2(width)-1:0] ipl_priority,
	output wire ipl_enable
	);
	
wire [width-1:0] status_int;
reg [width-1:0] intr_pending;
reg [width-1:0] intr_enable;
wire [width-1:0] intr_masked;
wire [$clog2(width)-1:0] intr_priority_y;
wire intr_priority_gs;

assign ipl_enable = intr_priority_gs;
assign ipl_priority = intr_priority_y;
	
assign intr_masked = intr_pending & intr_enable;
integer i;

mux4 #(width) UXB (
	.select(offset),
	.a0(intr_pending),
	.a1(intr_masked),
	.a2(intr_enable),
	.a3({intr_priority_gs, {(width-1-$clog(width)){1'b0}}, intr_priority_y}),
	.y(data_out)
	);

priority_encoder #(width) UXA (
	.a(intr_masked),
	.y(intr_priority_y),
	.group_select(intr_priority_gs)
	);

always@(posedge clock, negedge reset_n)
begin
	if(~reset_n)
	begin
		intr_pending <= 0;
		intr_enable <= 0;
	end
	else
	begin
		if(offset == 2'b00 & !rw_n)
			intr_pending <= intr_pending & ~data_in;
		else
		if(offset == 2'b01 & !rw_n)
			intr_pending <= intr_pending | data_in;
		else
		begin
			for(i = 0; i < width; i = i+1)
				if(irq_n[i]) intr_pending[i] <= 1;
		end
		
		if(offset == 2'b10 & !rw_n)
			intr_enable <= data_in;
	end
end
	
endmodule /* interrupt_controller */

	
//---------------------------------------------------------------------
//---------------------------------------------------------------------


module reset_driver(input wire reset_n, inout wire xreset_n, inout wire xhalt_n);

OPNDRN od_reset (.in(reset_n), .out(xreset_n));
OPNDRN od_halt  (.in(reset_n), .out(xhalt_n));

//assign xreset_n = ~reset_n ? 1'b0 : 1'bz;
//assign xhalt_n  = ~reset_n ? 1'b0 : 1'bz;		

endmodule /* reset_driver */


	
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
	input wire pcf_iack_n, // hack for fitter
		
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
	
	wire intc_ce;

	wire [7:0] status_hi;
	wire [7:0] databus_int;	
	wire [7:0] pio_page_oe;
	wire [7:0] status_page_8;
	wire [7:0] status_page_B;
	wire [7:0] status_pri;

	wire pio_page_oe_en;
	wire flash_ce;
	wire wram_ce;
	wire eeprom_ce;
	wire expansion_cs;
	wire pio_cs;	
	
	wire spi_write_wait;
	wire usb_write_wait;
	wire usb_read_wait;
	wire dtack_usb;
	wire txe_s;
	wire rxf_s;	
	wire timer0_ne;
	wire timer1_ne;
	wire timer2_ne;
	wire overlay_we;
	wire overlay_n;
	wire [3:0] intc_reg_we;
	wire ipri_gs;
	wire [2:0] ipri_y;
	wire [7:0] imasked;
	wire [7:0] ipend;
	wire [7:0] ienable;
	wire psw_debounced;
	wire psw_debounced_ne;
	wire spi_write;

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
	decoder #(8) U31 (
		.select(xadm[14:12]),
		.enable(pio_page_oe_en),
		.y(pio_page_oe)
		);
		
	/*	Mux IPEND, IMASKED, IENABLE, STATUS_PRI onto STATUS_PAGE_B */
	mux4 #(8) U1 (
		.select(xadl[7:6]),
		.a0(ipend),
		.a1(imasked),
		.a2(ienable),
		.a3(status_pri),
		.y(status_page_B)
		);
		
	wire xmlwr_n;
	wire xmuwr_n;

	/* 68000 and memory strobes */
	strobe_decode U11 (
		.as_n(xas_n),
		.lds_n(xlds_n),
		.uds_n(xuds_n),
		.rw_n(xrw_n),
		.lwr_n(xmlwr_n),
		.uwr_n(xmuwr_n),
		.rd_n(mrd_n),
		.fc(xfc),
		.vpa_n(xvpa_n)
		);
		
	wire status_dmago;
		
	// flash_ce_n
	// 
	assign mlwr_n = ~((~flash_ce_n & ~xmlwr_n & overlay_reg[3]) || (flash_ce_n & ~xmlwr_n));
	assign muwr_n = ~((~flash_ce_n & ~xmuwr_n & overlay_reg[3]) || (flash_ce_n & ~xmuwr_n));

	/* Memory decoder and Flash/WRAM overlay */	
	mem_decoder U91 (
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
	
	wire xpcf_cs_n;
	assign pcf_cs_n = 1'b1; // disable pcf
	
	/* PIO area decode */
	pio_strobe_decode U21(
		.cs(pio_cs),
		.rw_n(xrw_n),
		.adm(xadm[14:12]),
		.adl(xadl[7:6]),
		.post_ld_n(post_ld_n),
		.outp_ld_n(outp_ld_n),
		.pio2_ld_n(pio2_ld_n),
		.spi_write(spi_write),
		.uart_cs_n(uart_cs_n),
		.pit_cs_n(pit_cs_n),
		.pcf_cs_n(xpcf_cs_n),
		.usb_wr(usb_wr),
		.usb_rd_n(usb_rd_n),
		.intc_ce(intc_ce),
		.overlay_we(overlay_we)
		);

	/* Edge detector on PIT outputs (also synchronizes) */
	edge_detector IC50 (.clock(x8m), .reset_n(sysrst_n), .a(systick_1ms),  .ne(timer0_ne));	/* 1us pulse width */ 
	edge_detector IC51 (.clock(x8m), .reset_n(sysrst_n), .a(interval_1us), .ne(timer1_ne));	/* 1us pulse width */
	edge_detector IC52 (.clock(x8m), .reset_n(sysrst_n), .a(interval_1ms), .ne(timer2_ne));	/* 1ms pulse width */
	
	/* Synchronizer on UM245R TXE,RXF outputs */
	synchronizer  IC53 (.clock(x8m), .reset_n(sysrst_n), .a(usb_txe_n), .y(txe_s));
	synchronizer  IC54 (.clock(x8m), .reset_n(sysrst_n), .a(usb_rxf_n), .y(rxf_s));
	
	/* CPU signals */
	reset_driver U200 (.reset_n(sysrst_n), .xreset_n(xreset_n), .xhalt_n(xhalt_n));

	/* Decode interrupt page into four strobes (A7,A6) */
	decoder #(4) sub_dec (
		.select(xadl[7:6]),
		.enable(intc_ce & !xrw_n),
		.y(intc_reg_we)
		);

	/* Interrupt enable register */
	flopren #(8) U478 (.clock(x8m), .reset_n(sysrst_n), .enable(intc_reg_we[2]), .d(xdb), .q(ienable));
	
	/* Detect BREAK button with 1us resolution */
	edge_detector IC502 (.clock(systick_1ms), .reset_n(sysrst_n), .a(psw_break_n),  .ne(psw_debounced));	/* 1us pulse width */ 
	
	/* Detect BREAK button with 125ns resolution */
	edge_detector IC503 (.clock(x8m), .reset_n(sysrst_n), .a(psw_debounced),  .pe(psw_debounced_ne));	/* 1us pulse width */ 

	/* Overlay register */
	
	/*
		OVERLAY_REG
		D3 : FLASH_WE
		D2 : SPARE bit
		D1 : SPI SCLK
		D0 : SPI MOSI
	*/
	wire [3:0] overlay_reg;
	flopren #(4) U777 (.clock(x8m), .reset_n(sysrst_n), .enable(overlay_we), .d(xdb[3:0]), .q(overlay_reg));
		
	/* Interrupt set/reset/pending register  */
	flopren_rs #(4) U776 (
		.clock(x8m), 
		.reset_n(sysrst_n), 
		.clr_enable(intc_reg_we[0]), 
		.set_enable(intc_reg_we[1]), 
		.inputs({psw_debounced_ne, timer0_ne, timer1_ne, timer2_ne}),
		.d	(xdb[7:4]), 
		.q(ipend[7:4])
		);

	/* IPL priority encoder */
	priority_encoder #(8) U78 (
		.a(imasked),
		.y(ipri_y),
		.group_select(ipri_gs)
		);

   /* Read strobe enable for sub decode */
	assign pio_page_oe_en = pio_cs & xrw_n;	
	
	/* Status word */
	assign status_hi[0] = psw_break_n;
	assign status_hi[1] = spi_miso;
	assign status_hi[2] = status_dmago;
	assign status_hi[3] = uart_int_n;
	assign status_hi[4] = pcf_int_n;
	assign status_hi[5] = usb_pwe_n;
	assign status_hi[6] = usb_txe_n;
	assign status_hi[7] = usb_rxf_n;
	assign status_page_8 = status_hi;

	/* Pending bits are 1= Interrupt pending, 0= No interrupt pending */
	assign ipend[0] = 0;
	assign ipend[1] = 0; // pend sv?
	assign ipend[2] = 0;
	assign ipend[3] = ~uart_int_n;
	
	
	/* Status: Interrupt priority word */
	assign status_pri = {ipri_gs, 4'b0000, ipri_y[2:0]};
	
	/* DTACK */
	assign dtack_usb = usb_write_wait | usb_read_wait | spi_write_wait;
	assign usb_write_wait = sysrst_n & usb_wr & txe_s;
	assign usb_read_wait = sysrst_n & ~usb_rd_n & rxf_s;
	
	assign spi_write_wait = spi_write & status_dmago;
	
	// dtack_usb is combined wait condition
	// 
	
	assign xdtack_n = (dtack_usb)    ? 1'bz : 1'b0;	
	
	assign xdb = (pio_page_oe[3] 
	           ? status_page_B : 
				    pio_page_oe[0] ? 
					 status_page_8 : 8'bzzzzzzzz);
	
	/* Spare signals */
//	assign spi_mosi = overlay_reg[0];
//	assign spi_sclk = overlay_reg[1];
//	assign spare = overlay_reg[2] ^ spi_write;
	assign xipl_n = ipri_gs ? ~ipri_y : 3'b111;			
	assign imasked = ipend & ienable;

	
	spi_dma (
		.sysrst_n(sysrst_n),
		.clock(x8m),
		.start_strobe_we_n(spi_write),
		.config_strobe_we_n(xpcf_cs_n),
		.data(xdb[7:0]),
		.status_dmago(status_dmago),
		.spi_sclk(spi_sclk),
		.spi_mosi(spi_mosi),
		.spi_cs(spare),
		.spi_miso(spi_miso)
		);

endmodule /* sbc_glue */	

/* End */

//-------------------------------------------------------------
// Does a right shift of a register when clocked and enabled
// Does asynchronous load of parallel data
//-------------------------------------------------------------
module shift_register_left #(width = 8) (
	input wire clock,
	input wire enable,
	input wire load_n,
	input wire [width-1:0] din,
	input wire serial_in,
	output wire serial_out
	);
	
wire [width-1:0] dreg;
wire [width-1:0] dclr;
wire [width-1:0] dpre;
genvar i;

generate 
for(i = 0; i < width; i = i + 1)
begin : shift_chain
	wire source;
	assign source = i ? dreg[i-1] : serial_in;
	assign dpre[i] = !load_n &  din[i];
	assign dclr[i] = !load_n & ~din[i];	
	DFFE ff(
		.d(source), 
		.clk(clock),
		.clrn(dclr[i]), 
		.prn(dpre[i]), 
		.ena(enable), 
		.q(dreg[i])
		);	
end
endgenerate

assign serial_out = dreg[width-1];
	
endmodule /* shift_register_left */




module shift_register_right #(width = 8) (
	input wire clock,
	input wire enable,
	input wire load_n,
	input wire [width-1:0] din,
	input wire serial_in,
	output wire serial_out
	);
	
wire [width-1:0] dreg;
wire [width-1:0] dclr;
wire [width-1:0] dpre;
genvar i;

generate 
for(i = 0; i < width; i = i + 1)
begin : shift_chain
	wire source;
	assign source = (i == width-1) ? serial_in : dreg[i+1];
	assign dpre[i] = !load_n &  din[i];
	assign dclr[i] = !load_n & ~din[i];	
	DFFE ff(
		.d(source), 
		.clk(clock),
		.clrn(dclr[i]), 
		.prn(dpre[i]), 
		.ena(enable), 
		.q(dreg[i])
		);	
end
endgenerate

assign serial_out = dreg[0];
	
endmodule /* shift_register_right */




module spi_storage(
	input wire clock,
	input wire load_n,
	input wire enable,
	input wire [7:0] data_in,
	input wire serial_in,
	output wire serial_out
	);

	shift_register_left #(8) UXA0 (
		.clock(clock), 
		.enable(enable), 
		.load_n(load_n), 
		.din(data_in), 
		.serial_in(serial_in), 
		.serial_out(serial_out)
		);
	
	
endmodule


module spi_storage2 (
	input wire clock,
	input wire load_n, /* asynchronous */
	input wire enable,
	input wire [7:0] data_in,
	input wire serial_in,
	output wire serial_out
	);
	
reg [7:0] storage;

always@(posedge clock, negedge load_n)
begin
	if(~load_n)
	begin
		storage <= data_in;
	end
	else
	begin
		if(enable)
		begin
			storage <= {storage[6:0], serial_in};
		end
	end
	
end

	assign serial_out = storage[7];
	
endmodule /* spi_storage */














module spi_dma (
	input wire sysrst_n,
	input wire clock,
	input wire start_strobe_we_n,
	input wire config_strobe_we_n,
	input wire [7:0] data,
	output wire status_dmago,
	output wire spi_sclk,
	output wire spi_mosi,
	output wire spi_cs, 
	input wire spi_miso
	);
	
reg [4:0] cnt;
//wire [3:0] cnt;
reg dmago;
wire dmago_pe;
wire dmago_ne;

edge_detector IC501 (
	.clock(clock), 
	.reset_n(sysrst_n), 
	.a(start_strobe_we_n),  
	.pe(dmago_pe), 
	.ne(dmago_ne)
	);

// oops, start strobe we is negative pulse
// trailling edge of pulse is a positive edge
// use dmago_ne for posedge 1 clk pulse after write strobe done (Data valid)

/*
	getting some kind of spurious setting?
	
*/


assign spi_cs = ~dmago;//start_strobe_we_n;
assign spi_sclk = cnt[0];

wire clken;

assign clken = dmago && cnt > 0;

spi_storage IC502 (
	.clock(spi_sclk),
	.load_n(start_strobe_we_n),
	.enable(clken),
	.data_in(data),
	.serial_in(spi_miso),
	.serial_out(spi_mosi)
	);

/*
	need dmago to get assert after write is over
*/
	
// really we need a bit time before and after cs
// well, we will have manual cs anyway so maybe this isn't as important
assign status_dmago = dmago;
	
always@(posedge clock, negedge sysrst_n)
begin
	if(~sysrst_n)
	begin
		dmago <= 0;
		cnt <= 0;
	end
	else
	begin
		// here sysrst isn't asserted
		// if dmago is set we are counting
		
		// manual reset
		if(!config_strobe_we_n)
		begin
			cnt <= 0;
			dmago <= 0;
		end
		else
		begin
			if(dmago)
				cnt <= cnt + 5'd1;
			
			if(cnt == 5'b10000)
			begin
				dmago <= 0;
				cnt <= 0; /* required to make it retriggerable w/out manual clear  -- really need manual clear on write*/
			end
			else
			if(dmago_ne)
			begin
				dmago <= 1;
				cnt <= 0;  // this didn't fix
			end
		end
	end
end













/*
	
spi_storage IC502 (
	.clock(clock),
	.load_n(start_strobe_we_n),
	.enable(dmago),
	.data_in(data),
	.serial_in(spi_miso),
	.serial_out(spi_mosi)
	);

	
assign spi_sclk = cnt[0];

always@(posedge clock, negedge sysrst_n)
begin
	if(~sysrst_n)
	begin
		dmago <= 0;
	end
	else
	begin
		if(dmago_ne)
		begin
			dmago <= 1;
		end
		else
		if(cnt == 4'd15)
			dmago <= 0;
		else
			dmago <= dmago;
		
	end
end

assign status_dmago = dmago;
assign spi_cs = dmago;
	*/
endmodule








