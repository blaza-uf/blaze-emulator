#include "MemTypes.h"

class MemRam {
public:
    static constexpr u32 MEM_SIZE = 1024 * 128;
    Byte data[MEM_SIZE];
    void mem_init() {
        for (Byte & i : data) {
            i = 0;
        }
    }
    // Read data from memory
    Byte operator [] (u32 address) const {
        return data[address];
    }
    // Write data to memory
    Byte & operator [] (u32 address) {
        return data[address];
    }
};
