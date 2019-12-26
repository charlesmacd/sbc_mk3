
#ifndef _CPU_HPP_
#define _CPU_HPP_

#include <stdint.h>
#include "../newlib/printf.hpp"

typedef void (*IsrHandler)(void);

enum CPUVectorNumber : uint8_t
{
    AUTOVECTOR_LEVEL1    =   25,
    AUTOVECTOR_LEVEL2    =   26,
    AUTOVECTOR_LEVEL3    =   27,
    AUTOVECTOR_LEVEL4    =   28,
    AUTOVECTOR_LEVEL5    =   29,
    AUTOVECTOR_LEVEL6    =   30,
    AUTOVECTOR_LEVEL7    =   31,
};

struct interrupt_redirection_entry_tag
{
    uint16_t opcode;
    uint32_t address;
    uint16_t padding;
} __attribute__((packed, aligned(2)));

typedef struct interrupt_redirection_entry_tag interrupt_redirection_entry_t;

struct interrupt_vector_entry_tag
{
    uint8_t number;
    uint32_t address:24;
} __attribute__((packed, aligned(2)));

typedef struct interrupt_vector_entry_tag interrupt_vector_entry_t;

class cpu_68000
{
public:

    interrupt_vector_entry_t *get_vector_table_ptr(void)
    {
        return (interrupt_vector_entry_t *)0; /* VBR */
    }

    void set_interrupt_vector(CPUVectorNumber vector_number, interrupt_vector_entry_t vector)
    {
        interrupt_vector_entry_t *table = get_vector_table_ptr();
        table[vector_number] = vector;
    }

    interrupt_vector_entry_t get_interrupt_vector(CPUVectorNumber vector_number)
    {
        interrupt_vector_entry_t *table = get_vector_table_ptr();
        return table[vector_number];
    }





    interrupt_redirection_entry_t *get_redirection_table_ptr(void)
    {
        constexpr uint32_t kInterruptRedirectionTableBase = 0x100000;
        return (interrupt_redirection_entry_t *)kInterruptRedirectionTableBase;
    }    

    void set_interrupt_redirection_entry(CPUVectorNumber vector_number, interrupt_redirection_entry_t entry)
    {
        interrupt_redirection_entry_t *table = get_redirection_table_ptr();
        table[vector_number] = entry;
    }

    interrupt_redirection_entry_t *get_interrupt_redirection_entry(CPUVectorNumber vector_number)
    {
        interrupt_redirection_entry_t *table = get_redirection_table_ptr();
        return &table[vector_number];
    }


    IsrHandler get_redirected_isr(CPUVectorNumber vector_number)
    {
        return (IsrHandler)get_interrupt_redirection_entry(vector_number)->address;
    }

    void set_redirected_isr(CPUVectorNumber vector_number, IsrHandler handler)
    {
        interrupt_redirection_entry_t record;
        record.opcode = 0x4EF9;
        record.address = (uint32_t)handler;
        record.padding = 0x4E71;
        set_interrupt_redirection_entry(vector_number, record);
    }
};


#endif /* _CPU_HPP_ */