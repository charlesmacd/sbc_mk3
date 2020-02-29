

module top (

	/* Host interface */
	input wire hcs_n,
	input wire hwr_n,
	input wire hrd_n,
	input wire [1:0] ha,
	inout wire [7:0] hd,
	output wire hwait_n,
	output wire hint_n,
	
	/* Clock and reset */
	input wire reset_n,
	input wire clock,
	
	/* VRAM I/F */
	output wire [15:0] va,
	inout wire [7:0] vd,
	output wire vwe_n,
	output wire voe_n,
	output wire vce_n,
	
	/* Video I/F */
	output wire [5:0] dac_out,
	output wire hsync_n,
	output wire vsync_n,
	
	output wire debug
);

reg [8:0] hc;
reg [8:0] vc;
wire [15:0] host_pointer;
reg [15:0] scan_pointer;
reg [7:0] dac_latch;
wire hblank;
wire vblank;
wire cblank;

assign hsync_n = (hc >= 9'd240 && hc < 9'd248) ? 0 : 1;
assign vsync_n = (vc >= 9'd260 && vc < 9'd261) ? 0 : 1;
assign hblank  = (hc >= 9'd256);
assign vblank  = (vc >= 9'd224);
assign cblank = hblank | vblank;

assign voe_n = cblank ? 1 : 0;
assign vwe_n = 1'b1;
assign vce_n = 1'b0;

reg [7:0] reg_adl;
reg [7:0] reg_adh;
reg [7:0] reg_ctrl;
reg [7:0] host_data_latch;

assign host_pointer[7:0] = reg_adl;
assign host_pointer[15:8] = reg_adh;

always@(posedge hwr_n, negedge reset_n)
begin
	if(!reset_n)
	begin
		reg_adl <= 0;
		reg_adh <= 0;
		reg_ctrl <= 0;
		host_data_latch <= 0;
	end
	else
	begin
		case(ha[1:0])
		2'b00:
			reg_adl <= hd;
		2'b01:
			reg_adh <= hd;
		2'b10:
			begin
			host_data_latch <= hd;
			{reg_adh,reg_adl} <= {reg_adh,reg_adl} + 1;
			end
		2'b11:
			reg_ctrl <= hd;
		endcase
	end
end

assign vd = cblank ? 8'bzzzzzzzz : host_data_latch;

always@(posedge clock, negedge reset_n)
begin
	if(!reset_n)
	begin
		hc <= 9'd0;
		vc <= 9'd0;
	end
	else /* !reset_n */
	begin
		if(hc == 9'd342)
		begin
			hc <= 9'd0;
			if(vc == 9'd262)
			begin
				vc <= 9'd0;
				scan_pointer <= 0;
			end
			begin
				vc <= vc + 9'd1;
				if(vblank == 1)
				begin
					/* Bump row */
					scan_pointer[15:8] <= scan_pointer[15:8] + 8'd1;
				end
			end
		end
		else
		begin
			hc <= hc + 9'd1;
			dac_latch <= vd;
			if(hblank == 0)
			begin
				/* Bump column */
				scan_pointer[7:0] <= scan_pointer[7:0] + 8'd1;
			end
		end
	end
end

assign va = hcs_n ? scan_pointer : host_pointer;
assign dac_out[5:0] = {vc[2:0], hc[2:0]};

endmodule /**/








