#include "blaze/Bus.hpp"

namespace Blaze
{
    //=== Constructor ===
    Bus::Bus()
    : ram(),
    cpu()
    {
        // Connect to CPU on boot: reset
        cpu.reset(ram);
    }

    //=== Destructor ===
    Bus::~Bus(){}

    //=== Writing to the bus ===
    void Bus::write(Address addr, Byte data)
    {
        // Memory bounds check
        if(addr >= 0x0000 && addr <= 0xFFFF)
        {
            ram[addr] = data;
        }
    }

    //=== Reading from the bus ===
    Byte Bus::read(Address addr)
    {
        return ram[addr];
    }
}