#include <blaze/MemRam.hpp>
#include <blaze/util.hpp>

#include <cassert>

Blaze::MemRam::MemRam() {
	reset(nullptr);
}

void Blaze::MemRam::reset(Bus* bus) {
	for (Byte & i : data) {
		i = 0;
	}
};

Blaze::Byte Blaze::MemRam::registerSize(Address offset, Byte attemptedAccessSize) {
	return 8;
};

Blaze::Address Blaze::MemRam::read(Address offset, Byte bitSize) {
	assert(bitSize == 8);
	return data[offset];
};

void Blaze::MemRam::write(Address offset, Byte bitSize, Address value) {
	data[offset] = value;
};
