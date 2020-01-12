/*
	File:
		blocks.v
		
	Author:
		Charles MacDonald
		
	Notes:
		None
*/



module synchronizer (
	input wire clock,
	input wire reset_n,
	input wire a,
	output wire y
	);
	
	reg [1:0] history;
	
	always@(posedge clock, negedge reset_n)
	begin
		if(~reset_n)
			history <= 0;
		else
			history <= {history[0], a};
	end
	
	assign y = history[1];
	
endmodule

module edge_detector (
	input wire clock,
	input wire reset_n,
	input wire a,
	output wire pe,
	output wire ne
	);
	
	reg [1:0] history;
	
	always@(posedge clock, negedge reset_n)
	begin
		if(~reset_n)
			history <= 0;
		else
			history[1:0] <= {history[0] , a};			
	end
	
	assign pe =  history[1] & !history[0];
	assign ne = !history[1] &  history[0];
	
endmodule






module priority_encoder #(parameter width = 8) (
	input wire [width-1:0] a,
	output wire [$clog2(width)-1:0] y,
	output wire group_select
	);

	assign y = a[7] ? 3'd7
         : a[6] ? 3'd6
			: a[5] ? 3'd5
			: a[4] ? 3'd4
			: a[3] ? 3'd3
			: a[2] ? 3'd2
			: a[1] ? 3'd1
			: 3'd0
			;

	/* 37/128
always@(*)
begin
	casex(a)
		8'b1xxxxxxx: y = 3'd0;
		8'b01xxxxxx: y = 3'd1;
		8'b001xxxxx: y = 3'd2;
		8'b0001xxxx: y = 3'd3;
		8'b00001xxx: y = 3'd4;
		8'b000001xx: y = 3'd5;
		8'b0000001x: y = 3'd6;
		default:     y = 3'd7;
	endcase
end
*/
	
assign group_select = |a;			
	
endmodule










/* 2 to 1 mux */
module mux2 #(parameter width = 8) (
	input wire [1:0] select,
	input wire [width-1:0] a0,
	input wire [width-1:0] a1,
	output wire [width-1:0] y
	);
	
	assign y = select ? a1 : a0;
		
endmodule /* mux2 */










/* 4 to 1 mux */
module mux4 #(parameter width = 8) (
	input wire [1:0] select,
	input wire [width-1:0] a0,
	input wire [width-1:0] a1,
	input wire [width-1:0] a2,
	input wire [width-1:0] a3,
	output reg [width-1:0] y
	);
	
	always@(*)
	begin
		case(select)	
		3'd0: y <= a0;
		3'd1: y <= a1;
		3'd2: y <= a2;
	default: y <= a3;
		endcase
	end
		
endmodule /* mux4 */


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



/* ALU */
module alu #(parameter width = 8) (
	input wire [1:0] op,
	input wire [width-1:0] a,
	input wire [width-1:0] b,
	output reg [width-1:0] y
	);
	
	always@(*)
	begin
		case(op)
		2'd0: y <= a;
		2'd1: y <= a | b;
		2'd2: y <= a & ~b;
	default: y <= a ^ b;
		endcase
	end
		
endmodule /* alu */

/*
dffe <instance_name> (.d(<input_wire>), .clk(<input_wire>),

   .clrn(<input_wire>), .prn(<input_wire>), .ena(<input_wire>),

   .q(<output_wire>));
	
ena hi passes d to q
ena lo keeps q


DFF( .d .clk .clrn .prn .q)
DFFE( .d .clk .clrn .prn .q .ena)


	on clock update data
	on write clear

*/

module new_flopen_rs #(parameter width = 8) (
	input wire clock,
	input wire reset_n,
	input wire async_set_enable,
	input wire async_clr_enable,
	input wire [width-1:0] sync_set_inputs,
	input wire [width-1:0] d,
	output reg [width-1:0] q
	);
	
	wire [3:0] ff_clrn;
	wire [3:0] ff_prn;
	wire [3:0] ff_ena;

	assign ff_clrn[0] = ~( !reset_n | async_clr_enable & d[0] );
	assign ff_clrn[1] = ~( !reset_n | async_clr_enable & d[1] );
	assign ff_clrn[2] = ~( !reset_n | async_clr_enable & d[2] );
	assign ff_clrn[3] = ~( !reset_n | async_clr_enable & d[3] );
/*
	assign ff_clrn[4] = ~( !reset_n | async_clr_enable & d[4] );
	assign ff_clrn[5] = ~( !reset_n | async_clr_enable & d[5] );
	assign ff_clrn[6] = ~( !reset_n | async_clr_enable & d[6] );
	assign ff_clrn[7] = ~( !reset_n | async_clr_enable & d[7] );
	*/
	assign ff_prn[0] = ~( async_set_enable & d[0] );
	assign ff_prn[1] = ~( async_set_enable & d[1] );
	assign ff_prn[2] = ~( async_set_enable & d[2] );
	assign ff_prn[3] = ~( async_set_enable & d[3] );
	/*
	assign ff_prn[4] = ~( async_set_enable & d[4] );
	assign ff_prn[5] = ~( async_set_enable & d[5] );
	assign ff_prn[6] = ~( async_set_enable & d[6] );
	assign ff_prn[7] = ~( async_set_enable & d[7] );
	*/
	assign ff_ena[3:0] = 4'b1111;	

  DFFE R0 (.clk(clock), .clrn(ff_clrn[0]), .prn(ff_prn[0]), .d(sync_set_inputs[0]), .q(q[0]), .ena(ff_ena[0]) ); 
  DFFE R1 (.clk(clock), .clrn(ff_clrn[1]), .prn(ff_prn[1]), .d(sync_set_inputs[1]), .q(q[1]), .ena(ff_ena[1]) ); 
  DFFE R2 (.clk(clock), .clrn(ff_clrn[2]), .prn(ff_prn[2]), .d(sync_set_inputs[2]), .q(q[2]), .ena(ff_ena[2]) ); 
  DFFE R3 (.clk(clock), .clrn(ff_clrn[3]), .prn(ff_prn[3]), .d(sync_set_inputs[3]), .q(q[3]), .ena(ff_ena[3]) ); 
  /*
  DFFE R4 (.clk(clock), .clrn(ff_clrn[4]), .prn(ff_prn[4]), .d(sync_set_inputs[4]), .q(q[4]), .ena(ff_ena[4]) ); 
  DFFE R5 (.clk(clock), .clrn(ff_clrn[5]), .prn(ff_prn[5]), .d(sync_set_inputs[5]), .q(q[5]), .ena(ff_ena[5]) ); 
  DFFE R6 (.clk(clock), .clrn(ff_clrn[6]), .prn(ff_prn[6]), .d(sync_set_inputs[6]), .q(q[6]), .ena(ff_ena[6]) ); 
  DFFE R7 (.clk(clock), .clrn(ff_clrn[7]), .prn(ff_prn[7]), .d(sync_set_inputs[7]), .q(q[7]), .ena(ff_ena[7]) ); 
  */

endmodule /* debug */	

module flopren_rs #(parameter width = 4)(
	input wire clock,
	input wire reset_n,
	input wire set_enable,
	input wire clr_enable,
	input wire [width-1:0] inputs,
	input wire [width-1:0] d,
	output reg [width-1:0] q
	);
	
	integer i;
	
	always@(posedge clock, negedge reset_n)
	begin
		if(~reset_n)
			q <= {width{1'b0}};
		else
		begin
			if(clr_enable)
				q <= q & ~d;
			else
			if(set_enable)
				q <= q | d;
			else
			begin
					for(i = 0; i < width; i = i + 1)
						if(inputs[i]) q[i] <= 1;
			end
		end
	end	
	
endmodule /* flopren_rs */




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



/* I/O port with ALU */
module pio_port #(parameter width = 8) (
	input wire clock,
	input wire enable,
	input wire reset_n,
	input wire [1:0] alu_op,
	input wire [width-1:0] d,
	output wire [width-1:0] q
	);
	
	wire [width-1:0] alu_result;
	
	alu #(8) pio_alu (
		.op(alu_op),
		.a(q),
		.b(d),
		.y(alu_result)
		);
		
	flopren #(8) pio_flopren (
		.clock(clock),
		.reset_n(reset_n),
		.enable(enable),
		.d(alu_result),
		.q(q)
		);
		
endmodule /* pio_port */



/* 1 to N decoder */
module decoder #(parameter width = 4)(
	input wire [$clog2(width)-1:0] select,
	input wire enable,
	output wire [width-1:0] y
	);
	
	assign y = enable ? (1'b1 << select) : {width{1'b0}};

endmodule /* decoder */

/* 1 to N decoder, active low output */
module decoder_n #(parameter width = 4)(
	input wire [$clog2(width)-1:0] select,
	input wire enable,
	output wire [width-1:0] y
	);
	
	assign y = ~(enable ? (1'b0 << select) : {width{1'b1}});

endmodule /* decoder */



/* Tri-state driver */
module tristate_driver #(parameter width = 8) (
	input wire output_enable,
	input wire [width-1:0] a,
	inout wire [width-1:0] y
	);
	
assign y = (output_enable) ? a : {width{1'bz}};
	
endmodule /* tristate_driver */


/*----------------------------------------------------------*/
/* End */
/*----------------------------------------------------------*/


















module binary_counter #(parameter width = 3) (
	input wire clock,
	input wire reset_n,
	output wire [width-1:0] q
	);

reg [width-1:0] counter;
	
	
always@(posedge clock, negedge reset_n)
begin
	if(reset_n == 0)
		counter <= 'b0;
	else
		counter <= counter + 'b1;
end

assign q = counter;
	
endmodule

module binary_counter_limit #(parameter width = 4) (
	input wire clock,
	input wire reset_n,
	input wire [width-1:0] limit,
	output wire [width-1:0] q
	);

reg [width-1:0] counter;
	
wire [width-1:0] step;

assign step = 4'b0001;
	
always@(posedge clock, negedge reset_n)
begin
	if(~reset_n || counter == limit)
		counter <= {width{1'b0}};
	else
		counter <= counter + step;
end

assign q = width;
	
endmodule







module binary_counter_enable #(parameter width = 3) (
	input wire clock,
	input wire reset_n,
	input wire enable,
	output wire [width-1:0] q
	);

reg [width-1:0] counter;
	
	
always@(posedge clock, negedge reset_n)
begin
	if(reset_n == 0)
		counter <= 'b0;
	else
		counter <= (enable) ? (counter + 'b1) : counter;
end

assign q = width;
	
endmodule

