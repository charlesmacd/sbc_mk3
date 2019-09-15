/*
    File:
        usb.cpp
    Author:
        Charles MacDonald
    Notes:
        UM245R and DLP-USB245M interface
*/
#include <stdint.h>
#include <vector>
using namespace std;

#include "shared.h"
#include "usb.h"

#define FTDI_MAX_DEVICE_TYPES       13
#define BLOCK_SIZE                  0x8000

typedef struct
{
	DWORD flags;
	DWORD type;
	DWORD id;
	DWORD locid;
	char serial[64+1];
	char description[64+1];
	FT_HANDLE handle;
} ftdi_device_info;

FT_STATUS status;
FT_HANDLE handle;
DWORD device_no;
DWORD num_devices;


int usb_queue(void)
{
    DWORD amount = 0;
    status = FT_GetQueueStatus(handle, &amount);
    if(status != FT_OK)
    	die("Error: usb_queue(): FT_GetQueueStatus(): %s\n", usb_getstatus(status));
    return (int)amount;
}


void usb_get(uint8_t *buffer, uint32_t size)
{
    DWORD count;
    uint32_t i;
    uint32_t block_cnt = (size / BLOCK_SIZE);
    uint32_t block_rem = (size % BLOCK_SIZE);

    for(i = 0; i < block_cnt; i++)
    {
        size = BLOCK_SIZE;

        status = FT_Read(handle, buffer, size, &count);

        if(status != FT_OK)
            die("Read status (block %d of %d): %s\n", i, block_cnt-1, usb_getstatus(status));
    
        if(count != size)
            die("Read count (block %d of %d) was %d, expected %d\n", i, block_cnt-1, count, size);

        buffer += BLOCK_SIZE;
    }

    if(block_rem)
    {
        size = block_rem;

        status = FT_Read(handle, buffer, size, &count);

        if(status != FT_OK)
            die("Read status (block remainder): %s\n", i, block_cnt-1, usb_getstatus(status));
    
        if(count != size)
            die("Read count (block remainder) was %d, expected %d\n", i, block_cnt-1, count, size);
    }
}

void usb_send(uint8_t *buffer, uint32_t size)
{
    DWORD count;
    uint32_t i;
    uint32_t block_cnt = (size / BLOCK_SIZE);
    uint32_t block_rem = (size % BLOCK_SIZE);

    for(i = 0; i < block_cnt; i++)
    {
        size = BLOCK_SIZE;

        status = FT_Write(handle, buffer, size, &count);

        if(status != FT_OK)
            die("Write status (block %d of %d): %s\n", i, block_cnt-1, usb_getstatus(status));
    
        if(count != size)
            die("Write count (block %d of %d) was %d, expected %d\n", i, block_cnt-1, count, size);

        buffer += BLOCK_SIZE;
    }

    if(block_rem)
    {
        size = block_rem;

        status = FT_Write(handle, buffer, size, &count);

        if(status != FT_OK)
            die("Write status (block remainder): %s\n", i, block_cnt-1, usb_getstatus(status));
    
        if(count != size)
            die("Write count (block remainder) was %d, expected %d\n", i, block_cnt-1, count, size);
    }
}

void usb_reset(void)
{
    status = FT_ResetDevice(handle);
    if(status != FT_OK)
        die("Reset: status: %s\n", usb_getstatus(status));
}

void usb_purge(void)
{
      status = FT_Purge(handle, FT_PURGE_RX | FT_PURGE_TX);
      if(status != FT_OK)
          die("Purge: status: %s\n", usb_getstatus(status));
}

bool check_status(FT_STATUS status, char *msg)
{
	if(status != FT_OK)
	{
        die("Error: %s %s\n", msg, usb_getstatus(status));
        return false;
	}
	return true;        
}


const char *type_str[FTDI_MAX_DEVICE_TYPES] = 
{
	"BM",
	"AM",
	"100AX",
	"UNKNOWN",
	"2232C",
	"232R",
	"2232H",
	"4232H",
	"232H",
	"X-SERIES",
	"422H.0",
	"422H.1,2",
	"422H.3"
};

const char *ft_get_type(DWORD type)
{
	if(type >= FTDI_MAX_DEVICE_TYPES)
		return "Unknown";
	return type_str[type];		
}

void usb_init(bool list_devices)
{
	vector<int> compatible_device_list;
	DWORD device_count = 0;

	/* Get device list count */
	status = FT_CreateDeviceInfoList(&device_count);
	check_status(status, "usb_init(): FT_ListDevices():\n");

    if(!device_count)
        die("Error: No FTDI devices connected.\n");

	/* Scan through connected devices looking for compatible ones */
	printf("- Scanning for compatible devices.\n");
	for(int i = 0; i < device_count; i++)
	{
		DWORD flags;
		DWORD type;
		DWORD id;
		DWORD locid;
		char serial[16+1];
		char description[64+1];
		FT_HANDLE handle;

		/* Clear out information */
		flags = type = id = locid = 0;
		memset(description, 0, sizeof(description));
		memset(serial, 0, sizeof(serial));

		/* Initialize serial and description strings */
		sprintf(description, "(NO_DESCRIPTION)");
		sprintf(serial, "(NO_SERIAL)");
		
		/* Get device information */
		status = FT_GetDeviceInfoDetail(
			i, 
			&flags,
			&type,
			&id,
			&locid,
			serial,
			description,
			&handle
			);	

        if(list_devices)
        {
            printf("- Device %d, Description=[%s], Type=[%02X]\n", i, description, type);
        }

		/* Skip devices where we could not retrieve the information */
		if(status != FT_OK)
			continue;			

		/* Skip devices that are opened by another application (or not ready) */
		if(flags & FT_FLAGS_OPENED)
			continue;	

//        printf("- Querying device type %02X, desc [%s].\n", type, description);

		/* Skip devices that are not BM-type */
//		if(type != 0x04) // FIXME

		/* Skip BM-type devices that are not DLP-USB245M modules */
//		if(strcmp(description, "DLP-USB245M") != 0)		 // FIXME	
//		if(strcmp(description, "DLP232M A") != 0)		 // FIXME	
//		if(strcmp(description, "DLP2232M A") != 0)		 // FIXME	

        if(type != 0x05) // FIXME
            continue;           
        if(strcmp(description, "UM245R") != 0)      // FIXME   
			continue;


		/* Print device information */
		printf("Device #%d\n", i);
		printf("- Flags:       %08X (Opened:%s, High-Speed:%s)\n", 
			flags,
			(flags & FT_FLAGS_OPENED) ? "Yes" : "No",
			(flags & FT_FLAGS_HISPEED) ? "Yes" : "No"
			);
		printf("- Type:        %08X (%s)\n", type, ft_get_type(type));
		printf("- ID:          %08X\n", id);
		printf("- LocID:       %08X\n", locid);
		printf("- Serial Num:  %s\n", serial);
		printf("- Description: %s\n", description);
		printf("- Handle:      %08X\n", handle);

		/* At this point we have found a compatible device */
		compatible_device_list.push_back(i);
	}   

	/* Report how many compatible devices were found */
	if(compatible_device_list.empty())
	{
		printf("- No compatible devices found.\n");
		exit(1);
	}
	else
	{
		printf("- Found %d compatible devices.\n", compatible_device_list.size());
	}

	/* FIXME: Use first device in list */
	device_no = compatible_device_list[0];

	/* Open device */	
    status = FT_Open(device_no, &handle);
    if(status != FT_OK)
        die("Open status: %s\n", usb_getstatus(status));

	/* Print device information */
#if 1
		DWORD flags;
		DWORD type;
		DWORD id;
		DWORD locid;
		char serial[16+1];
		char description[64+1];
		FT_HANDLE handle;

		status = FT_GetDeviceInfoDetail(
			device_no, 
			&flags,
			&type,
			&id,
			&locid,
			serial,
			description,
			&handle
			);	
	
		printf("- Opened device #%d (%s)\n", device_no, description);
#endif	        

    /* Reset device */
    usb_reset();

    /* Flush buffers */
    usb_purge();
}


void usb_shutdown(void)
{
    /* Reset device */
    usb_reset();

    /* Flush buffers */
    usb_purge();

	/* Close handle */
    status = FT_Close(handle);
    if(status != FT_OK)
        die("Close status: %s\n", usb_getstatus(status));
}


char *usb_getstatus(FT_STATUS status)
{
    switch(status)
    {
        case FT_OK:
            return "OK";
        case FT_INVALID_HANDLE:
            return "Invalid handle";
        case FT_DEVICE_NOT_FOUND:
            return "Device not found";
        case FT_DEVICE_NOT_OPENED:
            return "Device not opened";
        case FT_IO_ERROR:
            return "I/O error";
        case FT_INSUFFICIENT_RESOURCES:
            return "Insufficient resouces";
        case FT_INVALID_PARAMETER:
            return "Invalid parameter";
        case FT_INVALID_BAUD_RATE:
            return "Invalid baud rate";
        case FT_DEVICE_NOT_OPENED_FOR_ERASE:
            return "Device not open for erase";
        case FT_DEVICE_NOT_OPENED_FOR_WRITE:
            return "Device not open for write";
        case FT_FAILED_TO_WRITE_DEVICE:
            return "Failed to write device";
        case FT_EEPROM_READ_FAILED:
            return "EEPROM read failed";
        case FT_EEPROM_WRITE_FAILED:
            return "EEPROM write failed";
        case FT_EEPROM_NOT_PRESENT:
            return "EEPROM not present";
        case FT_EEPROM_NOT_PROGRAMMED:
            return "EEPROM not programmed";
        case FT_INVALID_ARGS:
            return "Invalid args";
        case FT_NOT_SUPPORTED:
            return "Not supported";
        case FT_OTHER_ERROR:
            return "Other error";
    }
    return NULL;
}

void usb_logerror(int level, char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

/*--------------------------------------------------------------------------*/
/* Send data                                                                */
/*--------------------------------------------------------------------------*/

void usb_sendb(uint8_t value)
{
    uint8_t buffer[1];
    buffer[0] = value;
    usb_send(buffer, sizeof(buffer));
}

void usb_sendw(uint16_t value)
{
    uint8_t buffer[2];
    buffer[0] = (value >>  8) & 0xFF;
    buffer[1] = (value >>  0) & 0xFF;
    usb_send(buffer, sizeof(buffer));
}

void usb_sendl(uint32_t value)
{
    uint8_t buffer[4];
    buffer[0] = (value >> 24) & 0xFF;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >>  8) & 0xFF;
    buffer[3] = (value >>  0) & 0xFF;
    usb_send(buffer, sizeof(buffer));
}

/*--------------------------------------------------------------------------*/
/* Get data                                                                 */
/*--------------------------------------------------------------------------*/

uint8_t usb_getb(void)
{
    uint8_t value = 0;
    uint8_t buffer[1];
    usb_get(buffer, sizeof(buffer));
    value = buffer[0];    
    return value;
}

uint16_t usb_getw(void)
{
    uint16_t value = 0;
    uint8_t buffer[2];
    usb_get(buffer, sizeof(buffer));
    value |= (buffer[0] <<  0);
    value |= (buffer[1] <<  8);
    return value;
}

uint32_t usb_getl(void)
{
    uint32_t value = 0;
    uint8_t buffer[4];
    usb_get(buffer, sizeof(buffer));
    value |= (buffer[0] <<  0);
    value |= (buffer[1] <<  8);
    value |= (buffer[2] << 16);
    value |= (buffer[3] << 24);
    return value;
}
