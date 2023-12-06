#pragma once

#include <blaze/MMIO.hpp>

namespace Blaze {
	static constexpr Address MULDIV_BLOCK1_START = 0x4202;
	static constexpr Address MULDIV_BLOCK1_END = 0x4206;
	static constexpr Address MULDIV_BLOCK1_OFFSET_BEGIN = 0;
	static constexpr Address MULDIV_BLOCK2_START = 0x4214;
	static constexpr Address MULDIV_BLOCK2_END = 0x4217;
	static constexpr Address MULDIV_BLOCK2_OFFSET_BEGIN = 5;

	class MulDiv: public MMIODevice {
		Byte _mulA = 0xff;
		Byte _mulB = 0xff;
		Word _dividend = 0xffff;
		Byte _divisor = 0xff;
		Word _quotient = 0xffff;
		Word _productOrRemainder = 0xffff;

	public:
		Byte registerSize(Address offset, Byte attemptedAccessSize) override;
		Address read(Address offset, Byte bitSize) override;
		void write(Address offset, Byte bitSize, Address value) override;

		void reset(Bus* bus) override;
	};
};
