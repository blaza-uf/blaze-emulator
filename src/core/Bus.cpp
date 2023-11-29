#include "blaze/Bus.hpp"
#include <blaze/util.hpp>

static constexpr Blaze::Address BANK_SIZE = 0x010000;
static constexpr Blaze::Address BANK_HALF_SIZE = BANK_SIZE / 2;

static constexpr Blaze::Address LOROM_FINAL_64KIB = 0x3f0000;
static constexpr Blaze::Address HIROM_LINEAR_START = 0x400000;
static constexpr Blaze::Address HIROM_FINAL_128KIB_MEMORY_START = 0xfe0000;
static constexpr Blaze::Address HIROM_FINAL_128KIB_OFFSET_START = 0x3e0000;

static constexpr Blaze::Word LOWER_HALF_MIN = 0x0000;
static constexpr Blaze::Word LOWER_HALF_MAX = BANK_HALF_SIZE - 1;
static constexpr Blaze::Word UPPER_HALF_MIN = BANK_HALF_SIZE;
static constexpr Blaze::Word UPPER_HALF_MAX = 0xffff;

static constexpr bool addressIsUpperHalf(Blaze::Word address) {
	return address >= UPPER_HALF_MIN && address <= UPPER_HALF_MAX;
};

namespace Blaze
{
    //=== Constructor ===
    Bus::Bus()
    {
		// on startup, we reset all components
		reset();
    }

		Address Bus::read(Address address, Byte bitSize) {
			MMIODevice* device = nullptr;
			Address offset = 0;
			Address result = 0;
			Address resultShift = 0;

			while (bitSize > 0) {
				findDeviceAndOffset(address, bitSize, false, 0, device, offset);

				auto registerBitSize = device->registerSize(offset, bitSize);

				auto tmp = device->read(offset, registerBitSize);
				auto tmpPreserveMask = ~(UINT32_MAX << registerBitSize);
				auto resultPreserveMask = ~(UINT32_MAX << resultShift);

				result = (result & resultPreserveMask) | ((tmp & tmpPreserveMask) << resultShift);
				bitSize -= registerBitSize;
				resultShift += registerBitSize;
				address += registerBitSize / 8;
			}

			return result;
		};

		void Bus::write(Address address, Byte bitSize, Address data) {
			MMIODevice* device = nullptr;
			Address offset = 0;

			while (bitSize > 0) {
				findDeviceAndOffset(address, bitSize, true, data, device, offset);

				auto registerBitSize = device->registerSize(offset, bitSize);
				auto dataMask = ~(UINT32_MAX << registerBitSize);

				device->write(offset, registerBitSize, data & dataMask);

				data >>= registerBitSize;
				bitSize -= registerBitSize;
				address += registerBitSize / 8;
			}
		};

    //=== Writing to the bus ===
    void Bus::write(Address addr, Byte data)
    {
		write(addr, 8, data);
    }
    void Bus::write(Address addr, Word data)
    {
		write(addr, 16, data);
    }
    void Bus::write(Address addr, Address data)
    {
		write(addr, 24, data);
    }

    //=== Reading from the bus ===
    Byte Bus::read8(Address addr)
    {
		return read(addr, 8);
    }

    Word Bus::read16(Address addr)
    {
		return read(addr, 16);
    }

    Address Bus::read24(Address addr)
    {
		return read(addr, 24);
    }

	void Bus::reset() {
		ram.reset(this);
		// *don't* reset the ROM
		//rom.reset(this);
		dma.reset(this);
		if (ppu != nullptr) {
			ppu->reset(this);
		}
		if (apu != nullptr) {
			apu->reset(this);
		}
		cpu.reset(this);
	};

	struct DummyDevice: public MMIODevice {
		Address read(Address offset, Byte bitSize) override {
			return 0;
		};

		void write(Address offset, Byte bitSize, Address value) override {};

		void reset(Bus* bus) override {};
	};

	static DummyDevice globalDummyDevice;
}

void Blaze::Bus::findDeviceAndOffset(Address fullAddress, Byte bitSize, bool forWrite, Address valueWhenWriting, MMIODevice*& outDevice, Address& outOffset) {
	bool usingHiROM = rom.type() == ROM::Type::HiROM || rom.type() == ROM::Type::ExHiROM;

	Byte bank;
	Word addr;
	split24(fullAddress, bank, addr);

	outDevice = nullptr;
	outOffset = 0;

	// for both LoROM and HiROM, banks $80 through $FD are a mirror of banks $00 through $7D
	if (bank >= 0x80 && bank <= 0xfd) {
		bank -= 0x80;
	}

	if (bank == 0x00 && addr >= 0x2100 && addr <= 0x213f) {
		// the PPU has memory-mapped registers from $2100 through $213F
		outDevice = ppu;
		outOffset = addr - 0x2100;
		return;
	}

	if (bank == 0x00 && addr == 0x420b) {
		// DMA enable register
		outDevice = &dma;
		outOffset = DMA_SPECIAL_OFFSET_MDMAEN;
		return;
	}

	if (bank == 0x00 && addr == 0x420c) {
		// HDMA enable register
		outDevice = &dma;
		outOffset = DMA_SPECIAL_OFFSET_HDMAEN;
		return;
	}

	if (bank == 0x00 && addr >= 0x4300 && addr <= 0x437f) {
		// DMA control region
		outDevice = &dma;
		outOffset = addr - 0x4300;
		return;
	}

	if (bank == 0x00 && addr >= 0x2140 && addr <= 0x217f) {
		// APU IO registers (only 4 of them, but mirrored across this range)
		outDevice = apu;
		outOffset = addr % 4;
		return;
	}

	// banks $7E and $7F map the full 128 KiB of RAM
	if (bank == 0x7e || bank == 0x7f) {
		outDevice = &ram;
		outOffset = addr + ((bank == 0x7f) ? BANK_SIZE : 0);
		return;
	}

	// the first 2 pages of RAM are mirrored into the first 2 pages of every bank in banks $00 through $3F
	if (bank >= 0x00 && bank <= 0x3f && addr < 0x2000) {
		outDevice = &ram;
		outOffset = addr;
		return;
	}

	if (usingHiROM) {
		// in HiROM, the upper half of banks $00 through $3F map the corresponding ROM banks
		if (bank >= 0x00 && bank <= 0x3f && addressIsUpperHalf(addr)) {
			outDevice = &rom;
			// in this case, the corresponding offset is exactly the same as the full input address
			outOffset = fullAddress;
			return;
		}

		// in HiROM, banks $40 through $7D map the ROM out linearly
		if (bank >= 0x40 && bank <= 0x7d) {
			outDevice = &rom;
			// since this is mapped out linearly (full banks used), we can just subtract the start address to get the ROM offset
			outOffset = fullAddress - HIROM_LINEAR_START;
			return;
		}

		// in HiROM, banks $FE and $FF map the final 128 KiB of the ROM
		if (bank >= 0xfe && bank <= 0xff) {
			outDevice = &rom;
			// again: this is mapped out linearly (full banks used), so we can just subtract the start address (and add the offset start) to get the ROM offset
			outOffset = (fullAddress - HIROM_FINAL_128KIB_MEMORY_START) + HIROM_FINAL_128KIB_OFFSET_START;
			return;
		}
	} else {
		// in LoROM, the upper half of banks $00 through $7D map the ROM out linearly
		if (bank >= 0x00 && bank <= 0x7d && addressIsUpperHalf(addr)) {
			outDevice = &rom;
			outOffset = (addr - UPPER_HALF_MIN) + (bank * BANK_HALF_SIZE);
			return;
		}

		// in LoROM, the upper half of banks $FE and $FF map the final 64 KiB of the ROM
		if (bank >= 0xfe && bank <= 0xff && addressIsUpperHalf(addr)) {
			outDevice = &rom;
			outOffset = (addr - UPPER_HALF_MIN) + LOROM_FINAL_64KIB + ((bank == 0xfe) ? 0 : BANK_HALF_SIZE);
			return;
		}
	}

	// TODO:
	//   LoROM SRAM in lower half ($0000 through $7FFF) of banks $70 through $7D and banks $FE and $FF
	//   HiROM SRAM in $6000 through $7FFF of banks $20 through $3F
	//   all the SNES MMIO peripherals

	// if we got here, we were unable to map this access.
	outDevice = &globalDummyDevice;
	outOffset = 0;
	if (invalidAccess) {
		invalidAccess(fullAddress, bitSize, forWrite, valueWhenWriting);
	}
};
