#pragma once

#include <blaze/MMIO.hpp>

#include <array>

namespace Blaze {
	class APU: public MMIODevice {
	private:
		Bus* _bus = nullptr;
		std::array<Byte, 4> _portsFromCPU;
		std::array<Byte, 4> _portsToCPU;

	public:
		Byte registerSize(Address offset, Byte attemptedAccessSize) override;
		Address read(Address offset, Byte bitSize) override;
		void write(Address offset, Byte bitSize, Address value) override;

		void reset(Bus* bus) override;
	};
};
