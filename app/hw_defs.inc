#-----------------------------------------------------
# hw-defs.inc
#-----------------------------------------------------

USB_DATA                = 		0xFFFFA001
USB_STATUS              = 		0xFFFF8041
USB_RXF                 = 		7           
USB_TXE                 = 		6	        
USB_ENUM                = 		5

FLASH_BASE              =       0x000000
WRAM_BASE               =       0x100000
EPROM_BASE              =       0x200000
EXT_BASE                =       0x300000
PIO_BASE                =       0xFFFF8001

FLASH_SIZE              =       0x100000
EPROM_BASE              =       0x100000
WRAM_SIZE               =       0x100000

REG_POST                =       (PIO_BASE + 0x0000)
REG_STATUS              =       (PIO_BASE + 0x0040)

# Output latch #1

REG_OLAT0_BASE          =       (PIO_BASE + 0x0040)
REG_TIMER_1US_ENA       =       (REG_OLAT0_BASE + 0x00)
REG_TIMER_1MS_ENA       =       (REG_OLAT0_BASE + 0x02)
REG_UART_RESET          =       (REG_OLAT0_BASE + 0x04)
REG_USER_LED            =       (REG_OLAT0_BASE + 0x06)
REG_DP0                 =       (REG_OLAT0_BASE + 0x08)
REG_DP1                 =       (REG_OLAT0_BASE + 0x0A)
REG_DP2                 =       (REG_OLAT0_BASE + 0x0C)
REG_DP3                 =       (REG_OLAT0_BASE + 0x0E)

# Output latch #2

REG_OLAT1_BASE          =       (PIO_BASE + 0x0080)
REG_PCF_RESET           =       (REG_OLAT1_BASE + 0x00)  
REG_OD0                 =       (REG_OLAT1_BASE + 0x02)
REG_OD1                 =       (REG_OLAT1_BASE + 0x04)
REG_SPI_CS              =       (REG_OLAT1_BASE + 0x06)
REG_OUT3                =       (REG_OLAT1_BASE + 0x08)
REG_OUT2                =       (REG_OLAT1_BASE + 0x0A)
REG_OUT1                =       (REG_OLAT1_BASE + 0x0C)
REG_OUT0                =       (REG_OLAT1_BASE + 0x0E)

REG_UART_BASE           =       (PIO_BASE + 0x1000)
REG_PIT_BASE            =       (PIO_BASE + 0x1040)
REG_PCF_BASE            =       (PIO_BASE + 0x1080)
REG_PCF_IACK            =       (PIO_BASE + 0x10C0)

REG_USB_BASE            =       (PIO_BASE + 0x2000)
REG_MR                  =       (REG_UART_BASE + 0x00)

# Read registers
REG_SR              	=       (REG_UART_BASE + 0x02)
REG_BRG                 =       (REG_UART_BASE + 0x04)
REG_RHR                 =       (REG_UART_BASE + 0x06)
REG_TEST                =       (REG_UART_BASE + 0x08)
REG_ISR                 =       (REG_UART_BASE + 0x0A)
REG_CTU                 =       (REG_UART_BASE + 0x0C)
REG_CTL                 =       (REG_UART_BASE + 0x0E)

# Write registers
REG_CSR                 		=       (REG_UART_BASE + 0x02)
REG_CR                  		=       (REG_UART_BASE + 0x04)
REG_THR                 		=       (REG_UART_BASE + 0x06)
REG_ACR                 		=       (REG_UART_BASE + 0x08)
REG_IMR                 		=       (REG_UART_BASE + 0x0A)
REG_CTUR                		=       (REG_UART_BASE + 0x0C)
REG_CTLR                		=       (REG_UART_BASE + 0x0E)

B_STATUS_RECEIVED_BREAK 		=       7
B_STATUS_FRAMING_ERROR  		=       6
B_STATUS_PARITY_ERROR   		=       5
B_STATUS_OVERRUN        		=       4
B_STATUS_TXEMT          		=       3
B_STATUS_TXRDY          		=       2
B_STATUS_FIFO_FULL      		=       1
B_STATUS_RXRDY          		=       0

# CR[7:4] values:
CR_NOP                          =       0x00
CR_RESET_MR                     =       0x10
CR_RESET_RECEIVER               =       0x20
CR_RESET_TRANSMITTER            =       0x30
CR_RESET_ERROR_STATUS           =       0x40
CR_RESET_BREAK_CHANGE_INT       =       0x50
CR_START_BREAK                  =       0x60
CR_STOP_BREAK                   =       0x70
CR_START_CT                     =       0x80
CR_STOP_CT                      =       0x90
CR_ASSERT_RTS                   =       0xA0
CR_NEGATE_RTS                   =       0xB0
CR_RESET_MPI_CHANGE_INT         =       0xB0
CR_RESERVED_0                   =       0xC0
CR_RESERVED_1                   =       0xD0 /* Undocumented */
CR_RESERVED_2                   =       0xE0
CR_RESERVED_3                   =       0xF0

# CR[3:0] values:
CR_DISABLE_TRANSMITTER          =       0x08
CR_ENABLE_TRANSMITTER           =       0x04
CR_DISABLE_RECEIVER             =       0x02
CR_ENABLE_RECEIVER              =       0x01

AUTOVECTOR_ISR_NUMBER           =       25      

REG_IPENDING    				=       0xFFFFB001
REG_IPEND_CLR   				=       0xFFFFB001
REG_IPEND_SET   				=       0xFFFFB041
REG_IENABLE     				=       0xFFFFB081
REG_IMASKED     				=       0xFFFFB041
REG_IPRIORITY   				=       0xFFFFB0C1

F_IRQ_ACK_LVL1    				=       0x02
F_IRQ_ACK_LVL2    				=       0x04
F_IRQ_ACK_LVL3    				=       0x08
F_IRQ_ACK_LVL4    				=       0x10
F_IRQ_ACK_LVL5    				=       0x20
F_IRQ_ACK_LVL6    				=       0x40
F_IRQ_ACK_LVL7    				=       0x80

F_UART_IRQ_ACK    				=       F_IRQ_ACK_LVL3
B_UART_ISR_RXRDY  				=       2
B_UART_SR_RXRDY   				=       0
B_UART_ISR_TXEMT  				=       0
F_UART_IMR_TXEMT  				=       0x00
F_UART_IMR_RXRDY  				=       0x04

#-----------------------------------------------------
# End
#-----------------------------------------------------
