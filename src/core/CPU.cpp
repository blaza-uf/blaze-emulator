#include "CPU.h"

void CPU::reset() {
    PC = 0xFFFC; // need to load w/contents of reset vector
    DBR = PBR = 0x00;
    X = Y = 0x00;
    SP = 0x0100;

    f.d = 0;
    f.m = f.x = f.i = f.c = f.e = 1;
}

