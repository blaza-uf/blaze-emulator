#include <blaze/PPU.hpp>

struct PPUMMIORegister {
	enum IgnoreMe: Blaze::Address {
		INIDISP     = 0x2100,
		OBJSEL      = 0x2101,
		OAMADDL     = 0x2102,
		OAMADDH     = 0x2103,
		OAMDATA     = 0x2104,
		BGMODE      = 0x2105,
		MOSAIC      = 0x2106,
		BG1SC       = 0x2107,
		BG2SC       = 0x2108,
		BG3SC       = 0x2109,
		BG4SC       = 0x210A,
		BG12NBA     = 0x210B,
		BG34NBA     = 0x210C,
		BG1HOFS     = 0x210D,
		M7HOFS      = 0x210D,
		BG1VOFS     = 0x210E,
		M7VOFS      = 0x210E,
		BG2HOFS     = 0x210F,
		BG2VOFS     = 0x2110,
		BG3HOFS     = 0x2111,
		BG3VOFS     = 0x2112,
		BG4HOFS     = 0x2113,
		BG4VOFS     = 0x2114,
		VMAIN       = 0x2115,
		VMADDL      = 0x2116,
		VMADDH      = 0x2117,
		VMDATAL     = 0x2118,
		VMDATAH     = 0x2119,
		M7SEL       = 0x211A,
		M7A         = 0x211B,
		M7B         = 0x211C,
		M7C         = 0x211D,
		M7D         = 0x211E,
		M7X         = 0x211F,
		M7Y         = 0x2120,
		CGADD       = 0x2121,
		CGDATA      = 0x2122,
		W12SEL      = 0x2123,
		W34SEL      = 0x2124,
		WOBJSEL     = 0x2125,
		WH0         = 0x2126,
		WH1         = 0x2127,
		WH2         = 0x2128,
		WH3         = 0x2129,
		WBGLOG      = 0x212A,
		WOBJLOG     = 0x212B,
		TM          = 0x212C,
		TS          = 0x212D,
		TMW         = 0x212E,
		TSW         = 0x212F,
		CGWSEL      = 0x2130,
		CGADSUB     = 0x2131,
		COLDATA     = 0x2132,
		SETINI      = 0x2133,
		MPYL        = 0x2134,
		MPYM        = 0x2135,
		MPYH        = 0x2136,
		SLHV        = 0x2137,
		OAMDATAREAD = 0x2138,
		VMDATALREAD = 0x2139,
		VMDATAHREAD = 0x213A,
		CGDATAREAD  = 0x213B,
		OPHCT       = 0x213C,
		OPVCT       = 0x213D,
		STAT77      = 0x213E,
		STAT78      = 0x213F,
	};
};

Blaze::Address Blaze::PPU::read(Address offset, Byte bitSize) {
	// TODO
	return 0;
};

void Blaze::PPU::write(Address offset, Byte bitSize, Address value) {
	// TODO
};

Blaze::Byte Blaze::PPU::read8(Address offset) {
	return read(offset, 8);
};

Blaze::Word Blaze::PPU::read16(Address offset) {
	return read(offset, 16);
};

Blaze::Address Blaze::PPU::read24(Address offset) {
	return read(offset, 24);
};

void Blaze::PPU::write8(Address offset, Byte value) {
	write(offset, 8, value);
};

void Blaze::PPU::write16(Address offset, Word value) {
	write(offset, 16, value);
};

void Blaze::PPU::write24(Address offset, Address value) {
	write(offset, 24, value);
};

void Blaze::PPU::reset(Bus* bus) {
	_bus = bus;
};

void Blaze::PPU::beginVBlank() {

};

void Blaze::PPU::endVBlank() {

};
