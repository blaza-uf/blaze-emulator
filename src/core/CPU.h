#pragma once
#include <iostream>

class CPU{

    struct flags {
        // Process Status Flags
        unsigned c: 1; // carry
        unsigned z: 1; // zero
        unsigned i: 1; // interrupt disable
        unsigned d: 1; // decimal
        unsigned x: 1; // index register width
        unsigned b: 1; // break
        unsigned m: 1; // accumulator & memory width
        unsigned v: 1; // overflow
        unsigned n: 1; // negative

        unsigned e: 1; // emulation mode
    } ; flags f;

    using Byte = uint8_t;
    using Word = uint16_t;

    Word A; // accumulator
    Word DR; // direct
    Word PC; // program counter
    Word X, Y; // index registers
    Word SP; // stack pointer
    Byte DBR; // data bank
    Byte PBR; // program bank
    Byte P; // process status

    public:
        void reset();
};
