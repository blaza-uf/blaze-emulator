#pragma once

#include <blaze/MMIO.hpp>

#include <vector>

namespace Blaze {
	class SRAM: public MMIODevice {
		std::vector<Byte> _data;

	public:
		void setSize(size_t size);

		Byte registerSize(Address offset, Byte attemptedAccessSize) override;

		Address read(Address offset, Byte bitSize) override;
		void write(Address offset, Byte bitSize, Address value) override;

		void reset(Bus* bus) override;
	};
};
