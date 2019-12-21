`include "globals.inc"
`include "timebase.inc"

`define REG_PEND_CLR		0
`define REG_PEND_SET		1
`define REG_ENABLE		2
`define REG_VECTOR		3


	/* 
		D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0
		GS | 00 | 00 | 00 | 00 | P2 | P1 | P0
		GS : 1= P[2:0] contain valid level.
		   : 0= P[2:0] contain invalid level.
	*/
/*
	Of IRQ inputs currently:
	3:0 become Level 7,6,5,4
	  4 become Level 3
	Levels 2,1 not triggered.

	Register base is $FFB0x0
	
	00 : Write '1' to clear bits in pending register
	40 : Write '1' to set bits in pending register
	80 : Write '1' to enable pending bits to enter IPL priority encoder (masking)
	C0 : Not used
	
	00 : Read to get bitmask of pending interrupts
	40 : Read to get bitmask of enabled interrupts
	80 : Read to get bitmask of enable bits
	C0 : Read to get GS and priority of enabled inputs		
*/

module new_interrupt_controller (
	input wire clock,
	input wire reset_n,
	input wire chip_enable,
	input wire read_write_n,
	inout wire [7:0] host_din,
	output wire [7:0] host_qout,
	input wire [7:6] host_address,
	inout wire [2:0] host_ipl_n,
	input wire [7:0] irq // D[3:0] become IntLevel[7:4]
	);

	wire [7:0] intc_status_pri;
	wire [7:0] intc_masked;
	wire [7:0] intc_pending;
	wire [7:0] intc_enable;
	wire [7:0] intc_vector;
	wire [3:0] intc_reg_we;
	wire [2:0] intc_priority;
	wire intc_priority_gs;

	/* Decode write strobes at 00/40/80/C0 */
	decoder #(4) INTC_REG_DEC (
		.select(host_address[7:6]),
		.enable(chip_enable & !read_write_n),
		.y(intc_reg_we)
		);

	/* Interrupt enable register */
	flopren #(8) INTC_REG_ENABLE (
		.clock(clock), 
		.reset_n(reset_n), 
		.enable(intc_reg_we[`REG_ENABLE]), 
		.d(host_din), 
		.q(intc_enable)
		);
		
	/* Interrupt vector register */
	flopren #(8) INTC_REG_VECTOR (
		.clock(clock), 
		.reset_n(reset_n), 
		.enable(intc_reg_we[`REG_VECTOR]), 
		.d(host_din), 
		.q(intc_vector)
		);
		
	/* Pending interrupt register */
	flopren_rs #(4) INTC_REG_PENDING_RS (
		.clock(clock), 
		.reset_n(reset_n), 
		.clr_enable(intc_reg_we[`REG_PEND_CLR]), 
		.set_enable(intc_reg_we[`REG_PEND_SET]), 
		.inputs(irq[3:0]),
		.d	(host_din[7:4]), 
		.q(intc_pending[7:4])
		);

	/*	Mux readable registers to common status bus */
	mux4 #(8) INTC_REG_READ_MUX (
		.select(host_address[7:6]),
		.a0(intc_pending),
		.a1(intc_vector),
		.a2(intc_enable),
		.a3(intc_status_pri),
		.y(host_qout)
		);

	/* Define lower 4 bits of pending interrupt source (upper 4 are from RS flip-flop) */
	assign intc_pending[0] = 0; /* Not used */
	assign intc_pending[1] = 0; /* Level 1 */
	assign intc_pending[2] = 0; /* Level 2 */
	assign intc_pending[3] = ~irq[4]; /* Level 3 (UART) */	
	
	/* Mask pending interrupts by enable register */
	assign intc_masked = intc_pending & intc_enable;		

	/* Priority encoder */
	priority_encoder #(8) INTC_PRIORITY_ENCODER (
		.a(intc_masked),
		.y(intc_priority),
		.group_select(intc_priority_gs)
		);

	/* Form priority register status word */
	assign intc_status_pri = {intc_priority_gs, 4'b0000, intc_priority[2:0]};	
	
	/* Determine IPL output level */
	assign host_ipl_n = intc_priority_gs ? ~intc_priority : 3'b111;			
		
endmodule /* new_interrupt_controller */
