#include <blaze/MMIO.hpp>

Blaze::Byte Blaze::MMIODevice::registerSize(Address offset, Byte attemptedAccessSize) {
	return attemptedAccessSize;
};
