

#ifndef _PARALLEL_MEMORY_HPP_
#define _PARALLEL_MEMORY_HPP_

#include <stdint.h>
#include <string>
#include <map>
#include "../sys_types.hpp"
using namespace std;




class ParallelMemory
{
private:
public:

    uint32_t base_address;

    void set_base_address(uint32_t address)
    {
        base_address = address;
    }
};



enum EEPCommands : uint8_t
{
    kPageWrite      = 0xA0,
    kUnlockStep     = 0x80,
    kChipErase      = 0x10,
    kIDEntry        = 0x90,
    kIDExit         = 0xF0,
    kIDEntryAlt     = 0xA0
};



//----------------------------------



struct parallel_memory_device_information_tag
{
    string manufacturer;
    uint32_t size;
    uint32_t page_size;
};

//----------------------------------

typedef parallel_memory_device_information_tag parallel_memory_device_information_t;

//----------------------------------

class DevInfo
{
public:
    map<uint8_t, map<uint8_t, parallel_memory_device_information_t>> info;

    void initialize(void)
    {
        info[0xbf][0x07].manufacturer = "Greenliant";
        info[0xbf][0x07].size = 0x20000;
        info[0xbf][0x07].page_size = 0x80;
    }
};

//----------------------------------


class ParallelEEPROM
{
private:
public:

    /* Device configuration */
    uint32_t base_address;
    uint32_t page_size;

    uint8_t manufacturer_code;
    uint8_t device_code;
    uint32_t size;

    /* Memory configuration */
    volatile uint16_t *memory;
    uint8_t byte_lane;
    uint8_t shift;

    parallel_memory_device_information_t *info;

    ParallelEEPROM()
    {
        info = NULL;
    }

    void set_base_address(uint32_t address)
    {
        base_address = address;
        memory = (volatile uint16_t *)(address & ~1);
        byte_lane = address & 1;
        shift = byte_lane ? 8 : 0;
        size = 0;
    }

    uint32_t get_page_size(void)
    {
        return page_size;
    }

    bool page_write(uint32_t page_offset, uint8_t *data)
    {
        return true;
    }
    
    bool device_erase(void)
    {
        return true;
    }

    void write_command(uint8_t command)
    {
        constexpr uint32_t unlockAddr1 = 0x5555;
        constexpr uint8_t unlockData1 = 0xAA;

        constexpr uint32_t unlockAddr2 = 0x2AAA;
        constexpr uint8_t unlockData2 = 0x55;

        memory[unlockAddr1] = unlockData1 << shift;
        memory[unlockAddr2] = unlockData2 << shift;
        memory[unlockAddr1] = command << shift;

        /* delay or polling? */
    }

    void get_information(void)
    {
        /* Enter ID mode */
        write_command(EEPCommands::kIDEntry);
 
        /* Get ID */
        manufacturer_code = memory[0] >> shift;
        device_code = memory[1] >> shift;

        page_size = 0x80;
        size = 0x20000;

        /* Exit ID mode */
        write_command(EEPCommands::kIDExit);
    }
};


#endif /* _PARALLEL_MEMORY_HPP_ */