#include <blaze/APU.hpp>

#include <cassert>

Blaze::Byte Blaze::APU::registerSize(Address offset, Byte attemptedAccessSize) {
	return 8;
};

Blaze::Address Blaze::APU::read(Address offset, Byte bitSize) {
	assert(bitSize == 8);

	// TODO
	return 0;
};

void Blaze::APU::write(Address offset, Byte bitSize, Address value) {
	assert(bitSize == 8);

	// TODO
};

void Blaze::APU::reset(Bus* bus) {
	_bus = bus;
};
