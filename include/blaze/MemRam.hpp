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

		Byte registerSize(Address offset, Byte attemptedAccessSize) override;
		Address read(Address offset, Byte bitSize) override;
		void write(Address offset, Byte bitSize, Address value) override;

		void reset(Bus* bus) override;
	};
} // namespace Blaze
