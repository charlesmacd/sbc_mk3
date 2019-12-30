
#ifndef _APP_H_
#define _APP_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../L0_Platform/cpu_asm.hpp"
#include "../newlib/mem_heap.hpp"

enum ApplicationSection : uint8_t
{
	kTEXT       =   0,
	kDATA,
	kBSS,
	kKERNEL,
	kSTACK,
	kHEAP
};

struct section_info_tag
{
	uint32_t start;
	uint32_t end;
	uint32_t size;
	char name[32];
};

typedef section_info_tag section_info_t;


class Application
{
public:
    bool get_section_info(ApplicationSection which, section_info_t &record)
{
    extern char _text_start;
    extern char _etext;
    extern uint32_t _stext;
    extern char _data_start;
    extern char _edata;
    extern uint32_t _sdata;
    extern char _bss_start;
    extern char _bend;
    extern uint32_t _sbss;
    extern char _kernel_start;
    extern char _kernel_end;
    extern uint32_t _skernel;

    constexpr uint32_t kWorkRAMEnd = 0x200000; /* Get from SBC */
    constexpr uint32_t kStackTop   = 0x110000;

    switch(which)
    {
        case kTEXT:
            record.start = (uint32_t)&_text_start;
            record.end = (uint32_t)&_etext;
            record.size = (uint32_t)&_stext;
            strcpy(record.name, "TEXT");
            return true;

        case kDATA:
            record.start = (uint32_t)&_data_start;
            record.end = (uint32_t)&_edata;
            record.size = (uint32_t)&_sdata;
            strcpy(record.name, "DATA");
            return true;

        case kBSS:
            record.start = (uint32_t)&_bss_start;
            record.end = (uint32_t)&_bend;
            record.size = (uint32_t)&_sbss;
            strcpy(record.name, "BSS");
            return true;

        case kKERNEL:
            record.start = (uint32_t)&_kernel_start;
            record.end = (uint32_t)&_kernel_end;
            record.size = record.end - record.start;
            strcpy(record.name, "KERNEL");
            return true;

        case kHEAP:
            record.start = (uint32_t )sbrk(0);
            record.end = kWorkRAMEnd;
            record.size = record.end - record.start;
            strcpy(record.name, "HEAP");
            return true;

        case kSTACK:
            record.start = get_sp();
            record.end = kStackTop;
            record.size = record.end - record.start;
            strcpy(record.name, "STACK");
            return true;

        default:
            record.start = 0;
            record.end = 0;
            record.size = 0;
            strcpy(record.name, "<INVALID>");
            return false;
    }
}
};

#endif /* _APP_H_ */
