#pragma once
#include <iostream>
#include "MemRam.cpp"

struct CPU{

    enum flags {
        // Process Status Flags
        c = (1 << 0), // carry
        z = (1 << 1), // zero
        i = (1 << 2), // interrupt disable
        d = (1 << 3), // decimal
        x = (1 << 4), // index register width
        b = (1 << 4), // break
        m = (1 << 5), // accumulator & memory width
        v = (1 << 6), // overflow
        n = (1 << 7), // negative

    } ; flags f;

    Byte e = 1; //emulation mode. separate from p register flags

    Word A; // accumulator
    Word DR; // direct
    Word PC; // program counter
    Word X, Y; // index registers
    Word SP; // stack pointer
    Byte DBR; // data bank
    Byte PBR; // program bank
    Byte P; // process status


    void reset(MemRam &memory);
    void execute(u32 cTicks, MemRam &memory);

    void setFlag(flags f, bool s);

};
