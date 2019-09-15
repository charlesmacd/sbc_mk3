
#ifndef _FLASH_H_
#define _FLASH_H_

#include "sys_types.h"

void flash_program_sector(uint32_t address, uint8_t *buffer);
void flash_software_id_entry(void);
void flash_software_id_exit(void);
void flash_software_id_read(uint16_t *buf);

#endif /* _FLASH_H_ */

