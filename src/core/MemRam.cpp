#include <blaze/MemRam.hpp>
#include <blaze/util.hpp>

Blaze::MemRam::MemRam() {
	reset(nullptr);
}

void Blaze::MemRam::reset(Bus* bus) {
	for (Byte & i : data) {
		i = 0;
	}
};

Blaze::Byte Blaze::MemRam::read8(Address offset) {
	return data[offset];
};

Blaze::Word Blaze::MemRam::read16(Address offset) {
	return concat16(data[offset + 1], data[offset]);
};

Blaze::Address Blaze::MemRam::read24(Address offset) {
	return concat24(data[offset + 2], data[offset + 1], data[offset]);
};

void Blaze::MemRam::write8(Address offset, Byte value) {
	data[offset] = value;
};

void Blaze::MemRam::write16(Address offset, Word value) {
	split16(value, data[offset + 1], data[offset]);
};

void Blaze::MemRam::write24(Address offset, Address value) {
	split24(value, data[offset + 2], data[offset + 1], data[offset]);
};
