#pragma once

#include <blaze/MemTypes.hpp>
#include <array>

namespace Blaze {
	class MemRam {
		static constexpr uint32_t MEM_SIZE = 1024 * 128;

		std::array<Byte, MEM_SIZE> data;

	public:
		MemRam();

		void reset();

		// Read data from memory
		Byte operator [] (Address address) const;

		// Write data to memory
		Byte & operator [] (Address address);
	};
} // namespace Blaze
