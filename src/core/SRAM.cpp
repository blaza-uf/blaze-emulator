#include <blaze/SRAM.hpp>

#include <stdexcept>
#include <cassert>

void Blaze::SRAM::setSize(size_t size) {
	_data.resize(size, 0);
};

Blaze::Byte Blaze::SRAM::registerSize(Address offset, Byte attemptedAccessSize) {
	return 8;
};

Blaze::Address Blaze::SRAM::read(Address offset, Byte bitSize) {
	assert(bitSize == 8);

	offset %= _data.size();

	return _data[offset];
};

void Blaze::SRAM::write(Address offset, Byte bitSize, Address value) {
	assert(bitSize == 8);

	offset %= _data.size();

	_data[offset] = value;
};

void Blaze::SRAM::reset(Bus* bus) {
	std::fill(_data.begin(), _data.end(), 0);
};
