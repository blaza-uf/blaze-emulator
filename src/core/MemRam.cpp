#include <blaze/MemRam.hpp>

Blaze::MemRam::MemRam() {
	reset();
}

void Blaze::MemRam::reset() {
	for (Byte & i : data) {
		i = 0;
	}
};

// Read data from memory
Blaze::Byte Blaze::MemRam::operator [] (Address address) const {
	return data[address];
}

// Write data to memory
Blaze::Byte & Blaze::MemRam::operator [] (Address address) {
	return data[address];
}
