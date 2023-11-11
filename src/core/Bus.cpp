#include "blaze/Bus.hpp"

namespace Blaze
{
    //=== Constructor ===
    Bus::Bus()
    {
        cpu.bus = this;

        // Connect to CPU on boot: reset
        cpu.reset(ram);
    }

    //=== Writing to the bus ===
    void Bus::write(Address addr, Byte data)
    {
        // Memory bounds check
        if(addr >= 0x0000 && addr <= 0xFFFF)
        {
            ram[addr] = data;
        }
    }
    void Bus::write(Address addr, Word data)
    {
        // Memory bounds check
        if(addr >= 0x0000 && addr <= 0xFFFF)
        {
            ram[addr] = data;
        }
    }
    void Bus::write(Address addr, Address data)
    {
        // Memory bounds check
        if(addr >= 0x0000 && addr <= 0xFFFF)
        {
            ram[addr] = data;
        }
    }

    //=== Reading from the bus ===
    Byte Bus::read8(Address addr)
    {
        return ram[addr];
    }

    Word Bus::read16(Address addr)
    {
        return ram[addr];
    }

    Address Bus::read24(Address addr)
    {
        return ram[addr];
    }
}
