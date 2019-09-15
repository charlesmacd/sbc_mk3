

#include <stdint.h>
#include "sbc.h"

void cmd_echo(void)
{
	uint8_t ch;
	do
	{
		printf("Waiting for host ... ");
		ch = usb_getb();
		printf("Received %c (%02X), transmitting ...\n", ch, ch);
		usb_sendb(ch);
	} while(ch != ASCII_ESC);
}

enum {
	CMD_NOP				=	0x00,
	CMD_RESET,
	CMD_EXIT,
	CMD_EXEC,
	CMD_UPLOAD,
	CMD_DOWNLOAD,
	CMD_PEEKB,
	CMD_PEEKW,
	CMD_PEEKL,
	CMD_POKEB,
	CMD_POKEW,
	CMD_POKEL,
	CMD_DOWNLOAD_NOP,
	CMD_ECHO,
	CMD_ID,
	CMD_SYNC,
	CMD_UPLOAD_CHECKSUM,
	CMD_DOWNLOAD_CHECKSUM,
};

void cmd_download(void)
{
	uint32_t address, size;
	
	printf("Dispatch: Getting address...\n");
	address = usb_getl();
	printf("Dispatch: Address = %x\n", address);

	printf("Dispatch: Getting size...\n");
	size = usb_getl();
	printf("Dispatch: Size=%x\n", size);

	uint8_t *memptr = (uint8_t *)(address & 0x00FFFFFF);

	printf("Dispatch: Sending data...\n");
	usb_send(memptr, size);
	printf("Dispatch: Finished sending!\n");
}


void check_comms_dispatch(void)
{
	/* Check if there are any commands present */
	if(!usb_is_data_ready())
		return;

	printf("[COMMS] Data ready in RX FIFO, reading command.\n");

	/* Get command */
	uint8_t command = usb_getb();

	printf("[COMMS] Got command %x.\n", command);

	/* Dispatch command */
	switch(command)
	{
	case CMD_NOP:
		break;
	case CMD_RESET:
		soft_reboot();
		break;
	case CMD_EXIT:
		break;
	case CMD_EXEC:
		break;
	case CMD_UPLOAD:
		break;
	case CMD_DOWNLOAD:		
		printf("Dispatch: download\n");
		cmd_download();
		break;
	case CMD_PEEKB:
		break;
	case CMD_PEEKW:
		break;
	case CMD_PEEKL:
		break;
	case CMD_POKEB:
		break;
	case CMD_POKEW:
		break;
	case CMD_POKEL:
		break;
	case CMD_DOWNLOAD_NOP:
		break;
	case CMD_ECHO:
		cmd_echo();
		break;
	case CMD_ID:
		usb_sendb('R');
		break;
	case CMD_SYNC:
		break;
	case CMD_UPLOAD_CHECKSUM:
		break;
	case CMD_DOWNLOAD_CHECKSUM:
		break;
	default: /* Unknown command */
		uart_puts("ERROR: Unknown command received\n");
		hard_fault();
		break;
	}
}




