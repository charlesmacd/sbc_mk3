

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
    uint32_t base_address;
    uint32_t capacity;
    uint32_t mask;
    uint32_t shift;
    volatile uint16_t *memory;

public:

    void initialize(uint32_t dev_base_address, uint32_t dev_capacity)
    {
        base_address = dev_base_address;
        capacity = dev_capacity;

        /* Compute offset mask */
        mask = capacity - 1;
        
        /* Compute data shift */
        shift = (base_address & 1) ? 8 : 0;

        base_address &= ~1;

        /* Point to memory */
        memory = (volatile uint16_t *)base_address;
    }

    inline void write(uint32_t offset, uint8_t data)
    {
        memory[offset & mask] = data << shift;
    }

    inline uint8_t read(uint32_t offset)
    {
        return (memory[offset & mask] >> shift) & 0xFF;
    }

    inline uint32_t size(void)
    {
        return capacity;
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
    uint8_t byte_lane;

    uint8_t manufacturer_code;
    uint8_t device_code;
    uint32_t size;

    /* Memory configuration */
    volatile uint16_t *memory;
    int lane_width;
    uint8_t shift;

    parallel_memory_device_information_t *info;

    ParallelEEPROM()
    {
        info = NULL;
    }

    void set_base_address(uint32_t address, int width)
    {
        base_address = address;
        byte_lane = address & 1;
        memory = (volatile uint16_t *)(address & ~1);
        lane_width = width;
        shift = (address & 1) ? (width/2) : 0;
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
        constexpr uint32_t unlockAddr2 = 0x2AAA;

        if(lane_width == 16)
        {
            constexpr uint16_t unlockData1 = 0xAAAA;
            constexpr uint16_t unlockData2 = 0x5555;        
            memory[unlockAddr1] = unlockData1;
            memory[unlockAddr2] = unlockData2;
            memory[unlockAddr1] = (command << 8) | (command);
        }
        else
        {
            constexpr uint8_t unlockData1 = 0xAA;
            constexpr uint8_t unlockData2 = 0x55;        
            memory[unlockAddr1] = unlockData1 << shift;
            memory[unlockAddr2] = unlockData2 << shift;
            memory[unlockAddr1] = command << shift;
        }


        /* delay or polling? */
    }

    void get_information(void)
    {
        /* Enter ID mode */
        write_command(EEPCommands::kIDEntry);
 
        /* Get ID */

        if(lane_width == 16)
        {
            // validate
            manufacturer_code = (memory[0] >> 8) & 0xFF;
            device_code = (memory[1] >> 8) & 0xFF;

        }
        else
        {
        
            manufacturer_code = memory[0] >> shift;
            device_code = memory[1] >> shift;
        }

        page_size = 0x80;
        size = 0x20000;

        /* Exit ID mode */
        write_command(EEPCommands::kIDExit);
    }
};


#endif /* _PARALLEL_MEMORY_HPP_ */