#include <blaze/PPU.hpp>

struct PPUMMIORegister {
	enum IgnoreMe: Blaze::Address {
		INIDISP     = 0x00,
		OBJSEL      = 0x01,
		OAMADDL     = 0x02,
		OAMADDH     = 0x03,
		OAMDATA     = 0x04,
		BGMODE      = 0x05,
		MOSAIC      = 0x06,
		BG1SC       = 0x07,
		BG2SC       = 0x08,
		BG3SC       = 0x09,
		BG4SC       = 0x0A,
		BG12NBA     = 0x0B,
		BG34NBA     = 0x0C,
		BG1HOFS     = 0x0D,
		M7HOFS      = 0x0D,
		BG1VOFS     = 0x0E,
		M7VOFS      = 0x0E,
		BG2HOFS     = 0x0F,
		BG2VOFS     = 0x10,
		BG3HOFS     = 0x11,
		BG3VOFS     = 0x12,
		BG4HOFS     = 0x13,
		BG4VOFS     = 0x14,
		VMAIN       = 0x15,
		VMADDL      = 0x16,
		VMADDH      = 0x17,
		VMDATAL     = 0x18,
		VMDATAH     = 0x19,
		M7SEL       = 0x1A,
		M7A         = 0x1B,
		M7B         = 0x1C,
		M7C         = 0x1D,
		M7D         = 0x1E,
		M7X         = 0x1F,
		M7Y         = 0x20,
		CGADD       = 0x21,
		CGDATA      = 0x22,
		W12SEL      = 0x23,
		W34SEL      = 0x24,
		WOBJSEL     = 0x25,
		WH0         = 0x26,
		WH1         = 0x27,
		WH2         = 0x28,
		WH3         = 0x29,
		WBGLOG      = 0x2A,
		WOBJLOG     = 0x2B,
		TM          = 0x2C,
		TS          = 0x2D,
		TMW         = 0x2E,
		TSW         = 0x2F,
		CGWSEL      = 0x30,
		CGADSUB     = 0x31,
		COLDATA     = 0x32,
		SETINI      = 0x33,
		MPYL        = 0x34,
		MPYM        = 0x35,
		MPYH        = 0x36,
		SLHV        = 0x37,
		OAMDATAREAD = 0x38,
		VMDATALREAD = 0x39,
		VMDATAHREAD = 0x3A,
		CGDATAREAD  = 0x3B,
		OPHCT       = 0x3C,
		OPVCT       = 0x3D,
		STAT77      = 0x3E,
		STAT78      = 0x3F,
	};
};

Blaze::Byte Blaze::PPU::registerSize(Address offset, Byte attemptedAccessSize) {
	return 8;
};

Blaze::Address Blaze::PPU::read(Address offset, Byte bitSize) {
	// TODO
	return 0;
};

void Blaze::PPU::write(Address offset, Byte bitSize, Address value) {
	// TODO
};

void Blaze::PPU::reset(Bus* bus) {
	_bus = bus;
};

void Blaze::PPU::beginVBlank() {

};

void Blaze::PPU::endVBlank() {

};
