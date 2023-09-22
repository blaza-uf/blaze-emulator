#include "CPU.h"

void CPU::reset(MemRam &memory) {
    PC = 0xFFFC; // need to load w/contents of reset vector
    DBR = PBR = 0x00;
    X = Y = 0x00;
    SP = 0x0100;

    f.d = 0;
    f.m = f.x = f.i = f.c = f.e = 1;
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