#include <blaze/ROM.hpp>
#include <blaze/util.hpp>
#include <blaze/Bus.hpp>
#include <blaze/debug.hpp>

#include <fstream>
#include <cstring>
#include <cassert>

// 32 KiB
static constexpr size_t MIN_ROM_SIZE = 0x8000;
static constexpr size_t ROM_FIXED_VALUE = 0x33;
static constexpr size_t ROM_FIXED_VALUE_ALTERNATIVE = 0x01;
static constexpr size_t LOROM_HEADER_OFFSET = 0x007fb0;
static constexpr size_t HIROM_HEADER_OFFSET = 0x00ffb0;
static constexpr size_t LOROM_FIXED_VALUE_OFFSET = 0x007fda;
static constexpr size_t HIROM_FIXED_VALUE_OFFSET = 0x00ffda;
static constexpr size_t TITLE_SIZE = 21;

size_t Blaze::ROM::headerOffset() const {
	switch (_type) {
		case Type::LoROM:
		case Type::ExLoROM:
			return LOROM_HEADER_OFFSET;

		case Type::HiROM:
		case Type::ExHiROM:
			return HIROM_HEADER_OFFSET;

		default:
			return SIZE_MAX;
	}
};

Blaze::ROM::Type Blaze::ROM::type() const {
	return _type;
};

size_t Blaze::ROM::byteSize() const {
	if (_memory.empty()) {
		return 0;
	}

	return (static_cast<size_t>(1) << _memory[headerOffset() + HeaderFieldOffset::Size]) * 1024;
};

size_t Blaze::ROM::sramByteSize() const {
	if (_memory.empty()) {
		return 0;
	}

	switch (static_cast<CartridgeType>(_memory[headerOffset() + HeaderFieldOffset::CartridgeType])) {
		case CartridgeType::ROM_RAM:
		case CartridgeType::ROM_RAM_Battery:
		case CartridgeType::ROM_SA1_RAM:
		case CartridgeType::ROM_SA1_RAM_Battery:
			return (static_cast<size_t>(1) << _memory[headerOffset() + HeaderFieldOffset::RAMSize]) * 1024;

		default:
			return 0;
	}
};

std::string Blaze::ROM::name() const {
	if (_memory.empty()) {
		return {};
	}

	std::string result;
	result.resize(TITLE_SIZE, ' ');

	memcpy(result.data(), &_memory[headerOffset() + HeaderFieldOffset::GameTitle], TITLE_SIZE);

	return result;
};

void Blaze::ROM::load(const std::string& path) {
	_memory.clear();

	// open the file in binary mode and open it at the end (ATE) of the file to get the size
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	size_t size = file.tellg();

	// move the file back to the beginning
	file.seekg(0, std::ios::beg);

	_memory.resize(size);

	if (!file.read(reinterpret_cast<char*>(_memory.data()), size)) {
		throw std::runtime_error("failed to read ROM");
	}

	// determine the ROM type

	if (_memory.size() < MIN_ROM_SIZE) {
		// this is an invalid ROM
		throw std::runtime_error("ROM TOO SMALL: " + std::to_string(size));
		_memory.clear();
		_type = Type::INVALID;
		return;
	}

	// try to see if the LoROM header is valid
	if (_memory[LOROM_FIXED_VALUE_OFFSET] == ROM_FIXED_VALUE || _memory[LOROM_FIXED_VALUE_OFFSET] == ROM_FIXED_VALUE_ALTERNATIVE) {
		_type = Type::LoROM;
	}
	// try to see if the HiROM header is valid
	else if (_memory[HIROM_FIXED_VALUE_OFFSET] == ROM_FIXED_VALUE || _memory[LOROM_FIXED_VALUE_OFFSET] == ROM_FIXED_VALUE_ALTERNATIVE) {
		_type = Type::HiROM;
	} else {
		// invalid ROM
		_memory.clear();
		_type = Type::INVALID;
	}

	_bus->sram.setSize(sramByteSize());
};

Blaze::Byte Blaze::ROM::registerSize(Address offset, Byte attemptedAccessSize) {
	return 8;
};

Blaze::Address Blaze::ROM::read(Address offset, Byte bitSize) {
	if (_memory.empty()) {
		// no ROM loaded
		return 0;
	}

	assert(bitSize == 8);

	offset %= byteSize();

	return _memory[offset];
};

void Blaze::ROM::write(Address offset, Byte bitSize, Address value) {
	// no-op
	// this is read-only memory!
	Blaze::printLine("rom", "Attempt to write " + valueToHexString(value, 6, "$") + " to " + valueToHexString(offset, 6, "$") + " within ROM");
};

void Blaze::ROM::reset(Bus* bus) {
	_memory.clear();
	_type = Type::INVALID;
	_bus = bus;
};
