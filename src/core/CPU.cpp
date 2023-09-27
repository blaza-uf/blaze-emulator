#include "CPU.h"

void CPU::reset(MemRam &memory) {
    PC = 0xFFFC; // need to load w/contents of reset vector
    DBR = PBR = 0x00;
    X = Y = 0x00;
    SP = 0x0100;

    setFlag(d, 0);

    setFlag(m, 1);
    setFlag(x, 1);
    setFlag(i, 1);
    setFlag(c, 1);

    memory.mem_init();
}

void CPU::execute(u32 cycles, MemRam &memory) {
    while(cycles > 0) {
        Byte instruction = memory[PC];
        PC++;
        cycles--;

        // need to handle instructions
    }
}

void CPU::setFlag(CPU::flags flag, bool s) {
    if(s)
        P |= flag; // set flag
    else
        P &= ~flag; // clear flag
}
