
/*
	blocking:

	interrupt proc calls check dispatch
	don't want to run this during main application i/o

	comms_mutex




*/

#include <stdint.h>
#include "sbc.hpp"
#include "comms.hpp"
#include "L3_Application/mutex.hpp"
#include "L1_Peripheral/uart.hpp"
#include <string.h>

Mutex comms_mutex;

/*---------------------------------------*/


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
	CMD_NOP					=	0x00,
	CMD_RESET				=	0x01,
	CMD_EXIT				=	0x02,
	CMD_EXEC				=	0x03,
	CMD_UPLOAD				=	0x04,
	CMD_DOWNLOAD			=	0x05,
	CMD_PEEKB				=	0x06,
	CMD_PEEKW				=	0x07,
	CMD_PEEKL				=	0x08,
	CMD_POKEB				=	0x09,
	CMD_POKEW				=	0x0A,
	CMD_POKEL				=	0x0B,
	CMD_DOWNLOAD_NOP		=	0x0C,
	CMD_ECHO				=	0x0D,
	CMD_ID					=	0x0E,
	CMD_SYNC				= 	0x0F,

	CMD_UPLOAD_CHECKSUM		=	0x10,
	CMD_DOWNLOAD_CHECKSUM	=	0x11,

	PC_FOPEN				=	0x20,
	PC_FCLOSE				=	0x21,
	PC_FWRITE				=	0x22,
	PC_FREAD				=	0x23,
	PC_FTELL				=	0x24,
	PC_FSIZE				=	0x25,
	PC_FSEEK				=	0x26,
	PC_PUTS					=	0x27,
	PC_EXIT					=	0x28,
	PC_PRINTF				=	0x29,
};



int pc_fwrite(uint8_t handle, const uint8_t *buf, uint32_t size)
{
	usb_sendb(PC_FWRITE);
	usb_sendb(handle);
	usb_sendl(size);
	usb_send((uint8_t *)buf, size);
}

int pc_fopen(const char *filename, const char *mode)
{
	uint8_t result;

	usb_sendb(PC_FOPEN);
	usb_send_pstr(filename);
	usb_send_pstr(mode);
	result = usb_getb();
	return result;
}

void pc_fclose(uint8_t handle)
{
	usb_sendb(PC_FCLOSE);
	usb_sendb(handle);
}


void pc_puts(const char *msg)
{
	usb_sendb(PC_PUTS);
	usb_send_pstr(msg);
}

void pc_exit(void)
{
	usb_sendb(PC_EXIT);
}	

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
	if(!comms_mutex.attempt_lock())
	{
		/* The mutex was in use */
		return;
	}

	/* We have locked it now */

	/* Check if there are any commands present */
	if(!usb_is_data_ready())
	{
		comms_mutex.unlock();
		return;
	}

	/* Get command */
	uint8_t command = usb_getb();

	/* Dispatch command */
	switch(command)
	{
	case CMD_NOP:
		break;
	case CMD_RESET:
//		comms_mutex.unlock();
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
	case PC_FOPEN:
		break;
	case PC_FWRITE:
		break;
	case PC_FREAD:
		break;
	case PC_FTELL:
		break;
	case PC_FSIZE:
		break;
	case PC_FSEEK:
		break;
	case PC_PUTS:
		break;
	case PC_EXIT:
		break;
	case PC_PRINTF:
		break;

	default: /* Unknown command */
		printf("ERROR: Unknown command received\n");
		trigger_hard_fault();
		break;
	}

	comms_mutex.unlock();
}


/* End */
