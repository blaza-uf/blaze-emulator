#include <blaze/CPU.hpp>

bool Blaze::CPU::Register::using8BitMode() const {
	return (_flags & _mask) != 0;
};

void Blaze::CPU::Register::reset() {
	_value = 0;
};

Blaze::Word Blaze::CPU::Register::load() const {
	if (using8BitMode()) {
		return _value & 0xff;
	} else {
		return _value;
	}
};

void Blaze::CPU::Register::store(Word value) {
	if (using8BitMode()) {
		if (_mask == flags::m) {
			// the accumulator preserves the high byte
			_value = (_value & 0xff00) | (value & 0xff);
		} else {
			// the index registers clear the high byte
			_value = value & 0xff;
		}
	} else {
		_value = value;
	}
};

Blaze::Word Blaze::CPU::Register::forceLoadFull() const {
	return _value;
};

void Blaze::CPU::Register::forceStoreFull(Word value) {
	_value = value;
};

bool Blaze::CPU::Register::mostSignificantBit() const {
	if (using8BitMode()) {
		return (_value & (1u << 7)) != 0;
	} else {
		return (_value & (1u << 15)) != 0;
	}
};

Blaze::CPU::Register& Blaze::CPU::Register::operator+=(Word rhs) {
	store(load() + rhs);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator-=(Word rhs) {
	store(load() - rhs);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator*=(Word rhs) {
	store(load() * rhs);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator/=(Word rhs) {
	store(load() / rhs);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator&=(Word rhs) {
	store(load() & rhs);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator|=(Word rhs) {
	store(load() | rhs);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator^=(Word rhs) {
	store(load() ^ rhs);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator++() {
	store(load() + 1);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator++(int) {
	store(load() + 1);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator--() {
	store(load() - 1);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator--(int) {
	store(load() - 1);
	return *this;
};

Blaze::CPU::Register& Blaze::CPU::Register::operator=(Word rhs) {
	store(rhs);
	return *this;
};

bool Blaze::CPU::Register::operator==(Word rhs) const {
	return load() == rhs;
};

bool Blaze::CPU::Register::operator!=(Word rhs) const {
	return load() != rhs;
};

bool Blaze::CPU::Register::operator>=(Word rhs) const {
	return load() >= rhs;
};

bool Blaze::CPU::Register::operator<=(Word rhs) const {
	return load() <= rhs;
};

Blaze::Word Blaze::CPU::Register::operator+(Word rhs) const {
	return load() + rhs;
};

Blaze::Word Blaze::CPU::Register::operator-(Word rhs) const {
	return load() - rhs;
};

Blaze::Word Blaze::CPU::Register::operator*(Word rhs) const {
	return load() * rhs;
};

Blaze::Word Blaze::CPU::Register::operator/(Word rhs) const {
	return load() / rhs;
};

Blaze::Word Blaze::CPU::Register::operator&(Word rhs) const {
	return load() & rhs;
};

Blaze::Word Blaze::CPU::Register::operator|(Word rhs) const {
	return load() | rhs;
};

Blaze::Word Blaze::CPU::Register::operator^(Word rhs) const {
	return load() ^ rhs;
};
