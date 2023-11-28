#pragma once

#include <blaze/MemTypes.hpp>

namespace Blaze {
	struct Bus;

	// An abstract class (interface) for memory-mapped I/O devices
	class MMIODevice {
	public:
		virtual ~MMIODevice() = default;

		virtual Byte registerSize(Address offset, Byte attemptedAccessSize);

		virtual Address read(Address offset, Byte bitSize) = 0;
		virtual void write(Address offset, Byte bitSize, Address value) = 0;

		virtual void reset(Bus* bus) = 0;
	};
} // namespace Blaze
