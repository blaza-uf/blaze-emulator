#pragma once

#include <blaze/MMIO.hpp>

namespace Blaze {
	struct Bus;

	class PPU: public MMIODevice {
	private:
		Bus* _bus;

		Address read(Address offset, Byte bitSize);
		void write(Address offset, Byte bitSize, Address value);

	public:
		Byte read8(Address offset) override;
		Word read16(Address offset) override;
		Address read24(Address offset) override;

		void write8(Address offset, Byte value) override;
		void write16(Address offset, Word value) override;
		void write24(Address offset, Address value) override;

		void reset(Bus* bus) override;

		void beginVBlank();
		void endVBlank();
	};
};
