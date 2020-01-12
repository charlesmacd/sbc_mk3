/*	
	
	
	wire pio_clk_decode;
	wire pio_clk_wait;
	
	wire [7:0] addrbits;
	
	assign addrbits = {xadh[23:20], xadm[15:12]};
	
	// strobe for writing to 908000
	assign pio_clk_decode = !xas_n & (addrbits == 8'h98) & !xrw_n;
	
	reg [7:0] dtack_delay;
	
	// sreg is normally 0
	// during a pio clk cycle we start shifting in a 1 eventually
	always@(posedge x8m, negedge sysrst_n)
	begin
		if(!sysrst_n)
		begin
			dtack_delay <= 8'h00;
		end
		else
		begin
			dtack_delay <= {dtack_delay[6:0], pio_clk_decode};
		end
	end
	
	// if we aren't resetting
	// and we are accessing pio clk area
	// and the shift register hasn't yet filled with 1s, delay until it does
	assign pio_clk_wait = sysrst_n & pio_clk_decode & !dtack_delay[7];

*/
