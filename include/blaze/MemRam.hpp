#pragma once

#include <blaze/MemTypes.hpp>
#include <blaze/MMIO.hpp>
#include <array>

namespace Blaze {
	class MemRam: public MMIODevice {
		static constexpr uint32_t MEM_SIZE = 1024 * 128;

		std::array<Byte, MEM_SIZE> data;

	public:
		MemRam();

		Byte read8(Address offset) override;
		Word read16(Address offset) override;
		Address read24(Address offset) override;

		void write8(Address offset, Byte value) override;
		void write16(Address offset, Word value) override;
		void write24(Address offset, Address value) override;

		void reset(Bus* bus) override;
	};
} // namespace Blaze
