

#include "flash.hpp"

void flash_program_sector(uint32_t address, uint8_t *buffer)	
{
	printf("Status: Sending SDP enable command.\n");
	flash[0x5555] = 0xAAAA;
	flash[0x2AAA] = 0x5555;
	flash[0x5555] = 0xA0A0;
	delay_ms(10);


	printf("Status: Sending write command.\n");
	flash[0x5555] = 0xAAAA;
	flash[0x2AAA] = 0x5555;
	flash[0x5555] = 0xA0A0;

	
	printf("Status: Writing to page buffer.\n");

	for(int i = 0; i < 128; i++)
	{
		flash[0x4000+i] = 0xc0de;
	}

	printf("Status: Finished writing page.\n");
	printf("Status: Delay\n");

	delay_ms(1);

	
	printf("Status: Waiting for toggle\n");

	uint32_t counter = 0;
	uint16_t curr, last;
	bool toggling = true;
	while(toggling)
	{
		last = flash[0];
		curr = flash[0];
		if(last == curr)
			break;
		++counter;
		if(counter == 0x40000)
		{
			printf("Status: Toggle timeout.\n");
			toggling = false;
		}
	}
	printf("Status: Toggle normal exit, %x waits.\n", counter);
}

void flash_software_id_entry(void)
{
	flash[0x5555] = 0xAAAA;
	flash[0x2AAA] = 0x5555;
	flash[0x5555] = 0x9090;
}

void flash_software_id_exit(void)
{
	flash[0x5555] = 0xAAAA;
	flash[0x2AAA] = 0x5555;
	flash[0x5555] = 0xF0F0;
}

void flash_software_id_read(uint16_t *buf)
{
	buf[0] = flash[0];
	buf[1] = flash[1];
}
