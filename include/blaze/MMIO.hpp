#pragma once

#include <blaze/MemTypes.hpp>

namespace Blaze {
	class Bus;

	// An abstract class (interface) for memory-mapped I/O devices
	class MMIODevice {
	public:
		virtual ~MMIODevice() = default;

		virtual Byte read8(Address offset) = 0;
		virtual Word read16(Address offset) = 0;
		virtual Address read24(Address offset) = 0;

		virtual void write8(Address offset, Byte value) = 0;
		virtual void write16(Address offset, Word value) = 0;
		virtual void write24(Address offset, Address value) = 0;

		virtual void reset(Bus* bus) = 0;
	};
} // namespace Blaze
