#include <blaze/MulDiv.hpp>
#include <blaze/util.hpp>

struct MulDivMMIORegisters {
	enum IgnoreMe: Blaze::Byte {
		WRMPYA = Blaze::MULDIV_BLOCK1_OFFSET_BEGIN + 0,
		WRMPYB = Blaze::MULDIV_BLOCK1_OFFSET_BEGIN + 1,
		WRDIVL = Blaze::MULDIV_BLOCK1_OFFSET_BEGIN + 2,
		WRDIVH = Blaze::MULDIV_BLOCK1_OFFSET_BEGIN + 3,
		WRDIVB = Blaze::MULDIV_BLOCK1_OFFSET_BEGIN + 4,
		RDDIVL = Blaze::MULDIV_BLOCK2_OFFSET_BEGIN + 0,
		RDDIVH = Blaze::MULDIV_BLOCK2_OFFSET_BEGIN + 1,
		RDMPYL = Blaze::MULDIV_BLOCK2_OFFSET_BEGIN + 2,
		RDMPYH = Blaze::MULDIV_BLOCK2_OFFSET_BEGIN + 3,
	};
};

Blaze::Byte Blaze::MulDiv::registerSize(Address offset, Byte attemptedAccessSize) {
	return 8;
};

Blaze::Address Blaze::MulDiv::read(Address offset, Byte bitSize) {
	switch (offset) {
		case MulDivMMIORegisters::WRMPYA: return _mulA;
		case MulDivMMIORegisters::WRMPYB: return _mulB;
		case MulDivMMIORegisters::WRDIVL: return lo8(_dividend);
		case MulDivMMIORegisters::WRDIVH: return hi8(_dividend, true);
		case MulDivMMIORegisters::WRDIVB: return _divisor;
		case MulDivMMIORegisters::RDDIVL: return lo8(_quotient);
		case MulDivMMIORegisters::RDDIVH: return hi8(_quotient, true);
		case MulDivMMIORegisters::RDMPYL: return lo8(_productOrRemainder);
		case MulDivMMIORegisters::RDMPYH: return hi8(_productOrRemainder, true);

		default:
			return 0;
	}
};

void Blaze::MulDiv::write(Address offset, Byte bitSize, Address value) {
	switch (offset) {
		case MulDivMMIORegisters::WRMPYA:
			_mulA = value;
			break;
		case MulDivMMIORegisters::WRMPYB:
			_mulB = value;
			_productOrRemainder = static_cast<Word>(_mulA) * static_cast<Word>(_mulB);
			break;
		case MulDivMMIORegisters::WRDIVL:
			_dividend = hi8(_dividend, false) | value;
			break;
		case MulDivMMIORegisters::WRDIVH:
			_dividend = lo8(_dividend) | (value << 8);
			break;
		case MulDivMMIORegisters::WRDIVB:
			_divisor = value;
			if (_divisor == 0) {
				_quotient = 0xffff;
				_productOrRemainder = _dividend;
			} else {
				_quotient = _dividend / _divisor;
				_productOrRemainder = _dividend % _divisor;
			}
			break;

		default:
			break;
	}
};

void Blaze::MulDiv::reset(Bus* bus) {
	_mulA = 0xff;
	_mulB = 0xff;
	_dividend = 0xffff;
	_divisor = 0xff;
	_quotient = 0xffff;
	_productOrRemainder = 0xffff;
};
