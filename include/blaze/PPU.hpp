#pragma once

#include <blaze/MMIO.hpp>

namespace Blaze {
	struct Bus;

	class PPU: public MMIODevice {
	private:
		Bus* _bus;

	public:
		Byte registerSize(Address offset, Byte attemptedAccessSize) override;
		Address read(Address offset, Byte bitSize) override;
		void write(Address offset, Byte bitSize, Address value) override;

		void reset(Bus* bus) override;

		void beginVBlank();
		void endVBlank();
	};
};
