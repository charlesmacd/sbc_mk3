/*
    File: 
        main.cpp
    Author:
        Charles MacDonald
    Notes:
        None
*/

#include <string.h>
#include <strings.h>
#include <stdint.h>
#include "usb.h"
#include "shared.h"

/* Original command set */
#define CMD_NOP                 0x00
#define CMD_RESET               0x01
#define CMD_EXIT                0x02
#define CMD_EXEC                0x03
#define CMD_UPLOAD              0x04
#define CMD_DOWNLOAD            0x05
#define CMD_PEEKB				0x06
#define CMD_PEEKW				0x07
#define CMD_PEEKL				0x08
#define CMD_POKEB				0x09
#define CMD_POKEW				0x0A
#define CMD_POKEL				0x0B
#define CMD_DOWNLOAD_NOP		0x0C

/* Extended command set not supported by original BIOS */
#define CMD_ECHO                0x0D
#define CMD_ID                  0x0E
#define CMD_SYNC                0x0F
#define CMD_UPLOAD_CHECKSUM     0x10
#define CMD_DOWNLOAD_CHECKSUM   0x11

/* Dispatch commands */
#define PC_FOPEN        		0x20
#define PC_FCLOSE       		0x21
#define PC_FWRITE       		0x22
#define PC_FREAD        		0x23
#define PC_FTELL        		0x24
#define PC_FSIZE        		0x25
#define PC_FSEEK        		0x26
#define PC_PUTS         		0x27
#define PC_EXIT         		0x28
#define PC_PRINTF				0x29


/* Function prototypes */
int opt_game(int argc, char *argv[]);
int opt_reset(int argc, char *argv[]);
int opt_exit(int argc, char *argv[]);
int opt_nop(int argc, char *argv[]);
int opt_echo(int argc, char *argv[]);
int opt_upload(int argc, char *argv[]);
int opt_download(int argc, char *argv[]);
int opt_exec(int argc, char *argv[]);
int opt_debug(int argc, char *argv[]);
int opt_id(int argc, char *argv[]);
int opt_sync(int argc, char *argv[]);

/* Global variables */
int verbose = -1;
FILE *error_log = NULL;

/* Data */
static const option_t option_list[] = {
    {"-n",  &opt_nop},
    {"-r",  &opt_reset},
    {"-l",  &opt_upload},
    {"-d",  &opt_download},
    {"-x",  &opt_exec},
    {"-b",  &opt_exit},
    {"-e",  &opt_echo},
    {"-i",  &opt_id},
    {"-t",  &opt_debug},
    {"-s",  &opt_sync},
    {"-c",  &opt_dispatch},
    {NULL,  NULL},
};

void delay_ms(size_t count)
{
	Sleep(count);
}

int main (int argc, char *argv[])
{
    int i, j, k;


    

    /* Print help if insufficient arguments given */
    if(argc < 2)
    {
        printf("%s [-command]\n", argv[0]);
        printf("Commands are:\n");
        printf(" -r                          \tReset target.\n");
        printf(" -l <file.bin> <addr>        \tLoad file to target memory.\n");
        printf(" -d <file.bin> <addr> <size> \tSave target memory to file.\n");
        printf(" -x <file.bin> <addr>        \tLoad and execute program.\n");
        printf(" -c <file.bin> <addr>        \tLoad and execute program in dispatch mode.\n");
        printf("Diagnostic functions:\n");
        printf(" -n                          \tNo operation.\n");
        printf(" -e                          \tEcho test.\n");
        printf(" -i                          \tIdentify target source.\n");
        printf(" -s                          \tSynchronization test.\n");
        printf(" -b                          \tExit to BIOS.\n");
        printf(" -t                          \tTest.\n");
        
        return 1;
    }

    // scan list, open device based on user opts here    

    /* Set up error reporting */
    verbose = ERR_FATAL | ERR_WARN | ERR_INFO;
    error_log = stdout;

    /* Process commands */
    for(i = 1; i < argc; i++)
    {
        for(j = 0; option_list[j].key != NULL; j++)
        {
            if(stricmp(argv[i], option_list[j].key) == 0)
            {
                option_list[j].func(argc, argv);
            }
        }
    }

    return 0;
}

void die(char *format, ...)
{
    if(error_log && (verbose & ERR_FATAL))
    {
        va_list ap;
        va_start(ap, format);
        if(error_log) vfprintf(error_log, format, ap);
        va_end(ap);
    }

    exit(1);
}

void error(int level, char *format, ...)
{
    if(error_log && (verbose & level))
    {
        va_list ap;
        va_start(ap, format);
        if(error_log) vfprintf(error_log, format, ap);
        va_end(ap);
    }
}

/*--------------------------------------------------------------------------*/


void cmd_upload(uint8_t *buf, uint32_t address, uint32_t size)
{
    usb_sendb(CMD_UPLOAD);
    usb_sendl(address);
    usb_sendl(size);
    usb_send(buf, size);
}

void cmd_download(uint8_t *buf, uint32_t address, uint32_t size)
{
    usb_sendb(CMD_DOWNLOAD);
    usb_sendl(address);
    usb_sendl(size);
    usb_get(buf, size);
}

void cmd_exec(uint32_t address)
{
    usb_sendb(CMD_EXEC);
    usb_sendl(address);
    usb_sendb(0x01); // jmp a0
}

void cmd_reset(void)
{
	usb_sendb(CMD_RESET);
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

int opt_debug(int argc, char *argv[])
{
    printf("Startup USB\n");
    usb_init();

    for(int k = 0; k < 0x100; k++)
    {
        int temp = k;

        if((k & 0x0f) == 0x00)
            printf("%04X: ", k);

        usb_sendb(k);
        temp = usb_getb();

        printf("%02X ", temp);

        if((k & 0x0f) == 0x0f)
            printf("\n");
    }

    
    printf("Shutdown USB\n");
    usb_shutdown();
}

int opt_id(int argc, char *argv[])
{
    usb_init();
    error(ERR_INFO, "Getting ID: ");
    usb_sendb(CMD_ID);
    uint8_t target_id = usb_getb();
    printf("%c (%02X)\n", target_id, target_id);
    usb_shutdown();
}


int opt_sync(int argc, char *argv[])
{
    uint8_t temp;
    usb_init();

    error(ERR_INFO, "Synchronizing... ");

    usb_sendb(CMD_SYNC);
    
retry:
    temp = usb_getb();
    usb_sendb('D');
    
    if(temp != 'I')
		goto retry;

    temp = usb_getb();
    usb_sendb('K');
    
    if(temp != 'O')
		goto retry;
	    
    printf("finished.\n");

    usb_shutdown();
}


int opt_reset(int argc, char *argv[])
{
    usb_init();
    error(ERR_INFO, "CMD: Resetting target.\n");
    usb_sendb(CMD_RESET);
    Sleep(500);
    usb_shutdown();
}

int opt_exit(int argc, char *argv[])
{
    usb_init();
    error(ERR_INFO, "CMD: Exiting to BIOS.\n");
    usb_sendb(CMD_EXIT);
    usb_shutdown();
}

int opt_nop(int argc, char *argv[])
{
    usb_init();
    error(ERR_INFO, "CMD: No operation.\n");
    usb_sendb(CMD_NOP);
    usb_shutdown();
}


int opt_echo(int argc, char *argv[])
{
    uint8_t tx, rx;
    int running = 1;

    usb_init();
    usb_sendb(CMD_ECHO);

    printf("Echo test, press any key (ESC to quit).\n");
    while(running)
    {
        tx = getch();
        
        if(tx == 27)
            running = 0;
            
        printf("Key %02X (%c), ", tx, isprint(tx) ? tx : '_');
        usb_sendb(tx);

        printf("Send %02X (%c), ", tx, isprint(tx) ? tx : '_');
        rx = usb_getb();

        printf("Get %02X (%c)\n", rx, isprint(rx) ? rx : '_');
        if(tx != rx)
        {
            printf("Mismatch\n");
            exit(1);
        }
    }
    usb_shutdown();
}


/*
	1. Target sends address
	2. Target sends length
	3. Target sends data (no checksum)
*/
int opt_upload(int argc, char *argv[])
{
    FILE *fd;
    uint8_t *buffer;
    uint32_t address;
    uint32_t size;
    int left = argc - 2;

    /* Check parameters */
    if(left < 2)
        die("Insufficient arguments.\n");

    /* Convert parameters */
    address = strtoul(argv[3], NULL, 16);

    /* Range check */
    address &= 0xFFFFFFFF;

    /* Open input file */
    fd = fopen(argv[2], "rb");
    if(!fd)
        die("Couldn't open file `%s' for reading.\n", argv[2]);

    /* Get file size */
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    /* Check file size */
    if(!size)
    {
        fclose(fd);
        die("Invalid input file size (%d bytes).\n", (int)size);
        return 0;
    }

    /* Allocate buffer */
    buffer = (uint8_t *)malloc(size);
    if(!buffer)
    {
        die("Error allocating %d bytes.\n", (int)size);
        fclose(fd);
        return 0;
    }

    /* Clear buffer */
    memset(buffer, 0, size);

    /* Read file data */
    fread(buffer, size, 1, fd);
    fclose(fd);

    usb_init();

    /* Upload data */
    error(ERR_INFO, "Loading `%s' to $%08lX-$%08lX.\n", argv[2], address, (address + size - 1));

    cmd_upload(buffer, address, size);

    usb_shutdown();

    /* Free buffer */
    free(buffer);

    return 1;
}


int opt_download(int argc, char *argv[])
{
    FILE *fd;
    uint8_t *buffer;
	int count;
    uint32_t address;
    uint32_t size;
    int left = argc - 2;
    int target_checksum;
    int host_checksum;

    /* Check parameters */
    if(left < 3) {
        die("Insufficient arguments.\n");
        return 0;
    }

    /* Convert parameters */
    address = strtoul(argv[3], NULL, 16);
    size    = strtoul(argv[4], NULL, 16);


    /* Open input file */
    fd = fopen(argv[2], "wb");
    if(!fd)
    {
        die("Couldn't open file `%s' for writing.\n", argv[2]);
        return 0;
    }

    /* Allocate buffer */
    buffer = (uint8_t *)malloc(size);
    if(!buffer)
    {
        die("Error allocating %d bytes.\n", (int)size);
        fclose(fd);
        return 0;
    }

    /* Clear buffer */
    memset(buffer, 0, size);

    /* Download data */
    error(ERR_INFO, "- Saving target memory $%06lX-$%06lX to `%s'.\n", address, (address + size - 1), argv[2]);
    usb_init();
    cmd_download(buffer, address, size);


    /* Save to disk */
    count = fwrite(buffer, size, 1, fd);
    fclose(fd);

    /* Free buffer */
    free(buffer);

    usb_shutdown();

    return 1;
}



int opt_exec(int argc, char *argv[])
{
    FILE *fd;
    uint8_t *buffer;
    uint32_t address;
    uint32_t size;
    int left = argc - 2;

    /* Check parameters */
    if(left < 2)
        die("Insufficient arguments.\n");

    /* Convert parameters */
    address = strtoul(argv[3], NULL, 16);

    /* Open input file */
    fd = fopen(argv[2], "rb");
    if(!fd)
        die("Couldn't open file `%s' for reading.\n", argv[2]);

    /* Get file size */
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    /* Check file size */
    if(!size)
    {
        fclose(fd);
        die("Invalid input file size (%d bytes).\n", (int)size);
        return 0;
    }

    /* Allocate buffer */
    buffer = (uint8_t *)malloc(size);
    if(!buffer)
    {
        die("Error allocating %d bytes.\n", (int)size);
        fclose(fd);
        return 0;
    }

    /* Clear buffer */
    memset(buffer, 0, size);

    /* Read file data */
    fread(buffer, size, 1, fd);
    fclose(fd);

    /* Upload data */
    error(ERR_INFO, "Loading `%s' to $%08lX-$%08lX.\n", argv[2], address, (address + size - 1));
    usb_init();

	printf("-debug: upload cmd\n");
    usb_sendb(CMD_UPLOAD);
	printf("-debug: upload addr (%08X)\n", address);
    usb_sendl(address);
	printf("-debug: upload size (%08X)\n", size);
    usb_sendl(size);
	printf("-debug: upload data (%08X)\n", size);
    usb_send(buffer, size);

	// issue exec command (address and mode)
    usb_sendb(CMD_EXEC);
    usb_sendl(address);
    usb_sendb(0x01); // jmp a0

    usb_shutdown();

    /* Free buffer */
    free(buffer);

    return 1;
}








char *usb_get_pstr(void)
{
    int str_len;
    char *str;

    str_len = usb_getb();

    str = (char *)malloc(str_len+1);
    memset(str, 0, str_len+1);
    usb_get((uint8_t *)str, str_len);

    return str;
}    

FILE *file_handles[0x100];
    int handle_index = 0;

#define DEBUG   1

void pc_fread(void)
{
    FILE *fd;
    uint8_t handle = usb_getb();
    uint32_t size = usb_getl();
    uint32_t bytesRead = 0;

    printf("fread handle=%02X count=%08X\n", handle, size);
    
    fd = file_handles[handle];

    uint8_t *buffer = (uint8_t *)malloc(size);
    memset(buffer, 0x00, size);

    if(fd != NULL)
    {
        bytesRead = fread( buffer, 1, size, fd);
    }

    printf("read %d bytes of %d requested\n", bytesRead, size);

    usb_send(buffer, size);


    usb_sendl(bytesRead);

    free(buffer);

    printf("*** pc_fread:  handle=%02X size=%08X read=%08X\n",
        handle, size, bytesRead);
};

void pc_fwrite(void)
{
    FILE *fd;
    uint8_t handle = usb_getb();
    uint32_t size = usb_getl();
    uint32_t bytesWritten = 0;
    
    fd = file_handles[handle];

    printf("pc_fwrite: Request buffer size %08lX\n", size);
    uint8_t *buffer = (uint8_t *)malloc(size);
    if(!buffer)
    {
    	printf("pc_fwrite: Error allocating buffer.\n");
    	exit(1);
    }
    memset(buffer, 0, size);

	printf("pc_fwrite: Getting %08lX bytes from target.\n", size);
    usb_get(buffer, size);

	printf("pc_fwrite: Got data, flushing to disk.\n");
    if(fd != NULL)
    {
        bytesWritten = fwrite(buffer, 1, size, fd);
    }

	if(buffer)
	    free(buffer);

    printf("*** pc_fwrite: Finished: handle=%02X size=%08X read=%08X\n",
        handle, size, bytesWritten);
};


void pc_fopen(void)
{
    char *path = usb_get_pstr();
    char *mode = usb_get_pstr();
    FILE *fd;

    fd = fopen(path, mode);
    if(!fd)
    {
        usb_sendb(0x00);
        printf("*** pc_fopen:  handle=%02X str=%s mode=%s : open failed\n", handle_index, path, mode);
    }
    else
    {
        handle_index++;
        file_handles[ handle_index ] = fd;                        
        usb_sendb(handle_index);
        printf("*** pc_fopen:  handle=%02X str=%s mode=%s : open success\n", handle_index, path, mode);
    }

}

void pc_puts(void)
{
    char *str = usb_get_pstr();
    printf("*** pc_puts: [%s]\n", str);
    free(str);
}


enum states
{
	S_LITERAL,
	S_FORMAT,
};

bool ischr(char ch, char *list)
{
	for(int i = 0; i < strlen(list); i++)
		if(list[i] == ch)
			return true;
	return false;			
}

uint32_t swap(uint32_t data)
{
	return ((data >> 24) & 0x000000FF) |
		   ((data >>  8) & 0x0000FF00) |
		   ((data <<  8) & 0x00FF0000) |
		   ((data << 24) & 0xFF000000);
}

void pc_printf(void)
{
	char format[128];
	uint32_t *params = NULL;
	uint32_t size;
	char *str = NULL;
	int state = S_LITERAL;
	int width = -1;
	int leading = 0;
	int param_index = 0;

	/* Download formatted string */
	str = usb_get_pstr();

	printf("PC: ");	

	/* Download arguments */	
	size = usb_getl();
	if(size)
	{
		params = (uint32_t *)malloc(size);
		memset(params, 0, size);
		usb_get((uint8_t *)params, size);
	}

	/* Swap longword parameters from big-endian to little-endian */
	for(int i = 0; i < size/4; i++)
		params[i] = swap(params[i]);

	/* Process formatted strin g*/
	for(int i = 0; i < strlen(str); i++)
	{
		char ch = str[i];
		switch(state)
		{
			case S_LITERAL: /* Literal text */
				switch(ch)
				{
					case '%':
						state = S_FORMAT;
						leading = 0;
						width = 0;
						break;
					default:
						fputc(ch, stdout);
						break;
				}
				break;
				
			case S_FORMAT: /* Formatting */
				if(isdigit(ch))
				{
					if(width == 0 && ch == '0')
						leading = 1;
						
					if(isdigit(ch))
						width = (width * 10) + (ch-'0');
				}				
				else
				{
						switch(ch)
						{
							case 'd': /* Decimal */
							case 'b':					
								sprintf(format, "%%%02dd", width);
								fprintf(stdout, format, (signed long int)params[param_index++]);
								break;
						
							case 'x': /* Hex, lowercase */
								sprintf(format, "%%%02dx", width);
								fprintf(stdout, format, (uint32_t)params[param_index++]);
								break;
						
							case 'X': /* Hex, uppercase */
								sprintf(format, "%%%02dX", width);
								fprintf(stdout, format, (uint32_t)params[param_index++]);
								break;
						
							case 'c': /* Character */
								fprintf(stdout, "%c", (char)params[param_index++]);
								break;						

							default: /* Warn about unsupported format types */
								printf("Unknown format specified `%c'.\n", ch);
								break;
						}
						
						/* Resume literal mode */
						state = S_LITERAL;
				}
				break;
		}
	}

	/* Free allocated data */
	if(size)
		free(params);
	if(str)		
	 	free(str); 	
}

void pc_fclose(void)
{
    uint8_t handle = usb_getb();
    FILE *fd = file_handles[ handle ];

    if(fd != NULL)
    {
        fclose(fd);
        file_handles[handle] = NULL;
        --handle_index;
    }
    printf("*** pc_fclose: handle=%02X\n", handle);
//    usb_sendb(0x00);
}





void pc_fseek(void)
{
    FILE *fd;
    uint32_t position = 0;

    uint8_t handle = usb_getb();
    uint32_t offset = usb_getb();
    uint8_t mode = usb_getb();

    fd = file_handles[handle];

    if(fd != NULL)
    {
        fseek(fd, offset, mode);
        position = ftell(fd);
    }

    printf("*** pc_fseek: handle=%02X offset=%08X mode=%02X\n", handle, offset, mode);

    usb_sendl(position);
}


void pc_ftell(void)
{
    FILE *fd;
    uint32_t position = 0;

    uint8_t handle = usb_getb();
    fd = file_handles[handle];

    if(fd != NULL)
    {
        position = ftell(fd);
    }

    printf("*** pc_ftell: handle=%02X size=%08X\n", handle, position);

    usb_sendl(position);
}

void pc_fsize(void)
{
    FILE *fd;
    uint32_t size = 0;

    uint8_t handle = usb_getb();
    fd = file_handles[handle];

    if(fd != NULL)
    {
        fseek(fd, 0, SEEK_END);
        size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
    }

    printf("*** pc_fsize: handle=%02X size=%08X\n", handle, size);

    usb_sendl(size);
}



int opt_dispatch(int argc, char *argv[])
{
    FILE *fd;
    uint8_t *buffer;
    uint32_t address;
    uint32_t size;
    int left = argc - 2;


//////////////////////////////////////////
//////////////////////////////////////////

    /* Check parameters */
    if(left < 2)
        die("Insufficient arguments.\n");

    /* Convert parameters */
    address = strtoul(argv[3], NULL, 16);

    /* Open input file */
    fd = fopen(argv[2], "rb");
    if(!fd)
        die("Couldn't open file `%s' for reading.\n", argv[2]);

    /* Get file size */
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    /* Check file size */
    if(!size)
    {
        fclose(fd);
        die("Invalid input file size (%d bytes).\n", (int)size);
        return 0;
    }

    /* Allocate buffer */
    buffer = (uint8_t *)malloc(size);
    if(!buffer)
    {
        die("Error allocating %d bytes.\n", (int)size);
        fclose(fd);
        return 0;
    }

    /* Clear buffer */
    memset(buffer, 0, size);

    /* Read file data */
    fread(buffer, size, 1, fd);
    fclose(fd);

    /* Upload data */
    error(ERR_INFO, "Loading `%s' to $%08lX-$%08lX.\n", argv[2], address, (address + size - 1));
    usb_init();

	printf("-debug: upload cmd\n");
    usb_sendb(CMD_UPLOAD);
	printf("-debug: upload addr (%08X)\n", address);
    usb_sendl(address);
	printf("-debug: upload size (%08X)\n", size);
    usb_sendl(size);
	printf("-debug: upload data (%08X)\n", size);
    usb_send(buffer, size);

	// issue exec command (address and mode)
    usb_sendb(CMD_EXEC);
    usb_sendl(address);
    usb_sendb(0x01); // jmp a0

//////////////////////////////////////////
//////////////////////////////////////////

    printf("Entering dispatch loop.\n");


    for(int i = 0; i < 0x100; i++)
        file_handles[i] = NULL;


    int running = 1;
    while(running)
    {
        uint8_t cmd = usb_getb();

        switch(cmd)
        {
            case PC_FSIZE:
                pc_fsize();
                break;
        
            case PC_FTELL:
                pc_ftell();
                break;
        
            case PC_FSEEK:
                pc_fseek();
                break;
        
            case PC_FREAD:
                pc_fread();
                break;
        
            case PC_FWRITE:
                pc_fwrite();
                break;

            case PC_FOPEN:
                pc_fopen();
                break;
        
            case PC_FCLOSE:
                pc_fclose();
                break;
        
            case PC_PUTS:
                pc_puts();
                break;
        
            case PC_EXIT:
                printf("*** pc_exit\n");
                running = 0;
                break;   

		case PC_PRINTF:
			pc_printf();
			break;                

            default:
                printf("*** Dispatch, unknown command %02X\r", cmd);
                break;                
        }
    }


    usb_shutdown();

    /* Free buffer */
    free(buffer);

    return 1;
}





