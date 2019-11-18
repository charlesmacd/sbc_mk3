#-------------------------------------------------------------------------------
# cpu_asm.inc
#-------------------------------------------------------------------------------

# Amount of WRAM to set aside for kernel
WRAM_RESERVED           =       0x10000

# Location of ISR table
ISR_BASE                =       WRAM_BASE

# Address to load RAM-resident programs into 
LOAD_ADDRESS            =       (WRAM_BASE + WRAM_RESERVED)

# Kernal stack top address
LOCAL_STACK_TOP         =       (WRAM_BASE + WRAM_RESERVED)

# Vector table location
VECTOR_TABLE_BASE       =       0x0000             

# Opcode for "JMP $mem32"
OPCODE_JMP_ABS32        =       0x4EF9   

F_EXTEND                =       0x10
F_CARRY                 =       0x01

# Vector indices for autovector interrupts
AUTOVECTOR_LEVEL1_NUMBER    =   25
AUTOVECTOR_LEVEL2_NUMBER    =   26
AUTOVECTOR_LEVEL3_NUMBER    =   27
AUTOVECTOR_LEVEL4_NUMBER    =   28
AUTOVECTOR_LEVEL5_NUMBER    =   29
AUTOVECTOR_LEVEL6_NUMBER    =   30
AUTOVECTOR_LEVEL7_NUMBER    =   31

#-------------------------------------------------------------------------------
# End
#-------------------------------------------------------------------------------
