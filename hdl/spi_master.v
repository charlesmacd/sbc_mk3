/*

	on write to address, 
	1. load register with word using set/clr
	2. load cnt with 8
	3. load dmago with 1
	
	load/shift is load during cpu write pulse, then shift
	on shift use clk/d/q to rotate data and bring in new data
	
	load_shift = 

	
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

*/




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


