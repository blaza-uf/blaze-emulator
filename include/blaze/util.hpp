#pragma once

#include <cstdint>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace Blaze {
	// NOLINTBEGIN(readability-magic-numbers, readability-identifier-length, bugprone-easily-swappable-parameters)
	static constexpr uint16_t concat16(uint8_t hi, uint8_t lo) {
		return (static_cast<uint16_t>(hi) << 8) | lo;
	};

	static constexpr uint32_t concat24(uint8_t hi, uint16_t lo) {
		return (static_cast<uint32_t>(hi) << 16) | lo;
	};

	static constexpr uint32_t concat24(uint8_t hi, uint8_t mid, uint8_t lo) {
		return concat24(hi, concat16(mid, lo));
	};

	static constexpr void split16(uint16_t val, uint8_t& hi, uint8_t& lo) {
		hi = static_cast<uint8_t>((val >> 8) & 0xff);
		lo = static_cast<uint8_t>(val & 0xff);
	};

	static constexpr void split24(uint32_t val, uint8_t& hi, uint16_t& lo) {
		hi = static_cast<uint8_t>((val >> 16) & 0xff);
		lo = static_cast<uint16_t>(val & 0xffff);
	};

	static constexpr void split24(uint32_t val, uint8_t hi, uint8_t mid, uint8_t lo) {
		uint16_t tmp = 0;
		split24(val, hi, tmp);
		split16(tmp, mid, lo);
	};

	static constexpr bool msb8(uint8_t value) {
		return (value & 0x80) != 0;
	};

	static constexpr bool msb16(uint16_t value) {
		return (value & 0x8000) != 0;
	};

	/**
	 * Like `msb8()` and `msb16()`, except you can choose whether the value is 8-bit or 16-bit at runtime.
	 *
	 * If `get8BitMSB` is false, returns the 16-bit MSB.
	 * If `get8BitMSB` is true, returns the 8-bit MSB.
	 */
	static constexpr bool msb(uint16_t value, bool get8BitMSB) {
		return get8BitMSB ? msb8(value) : msb16(value);
	};

	template<typename T>
	static constexpr T lo8(T value) {
		return value & static_cast<T>(0xff);
	};

	template<typename T>
	static constexpr T lo16(T value) {
		return value & static_cast<T>(0xffff);
	};

	template<typename T>
	static constexpr T lo(T value, bool limitTo8Bits) {
		return limitTo8Bits ? lo8(value) : lo16(value);
	};

	template<typename T>
	static constexpr T mid8(T value, bool shift) {
		auto unshifted = value & static_cast<T>(0xff00);
		return shift ? (unshifted >> 8) : unshifted;
	};

	template<typename T>
	static constexpr T hi8(T value, bool shift) {
		// treat types greater than 16 bits as 24-bit types (since the CPU can only handle up to 24 bit types)
		size_t typeBits = (sizeof(value) > 2) ? 24 : (sizeof(T) * 8);
		auto shiftBits = typeBits - 8;
		auto unshifted = value & (static_cast<T>(0xff) << shiftBits);
		return shift ? (unshifted >> shiftBits) : unshifted;
	};

	template<typename T>
	static constexpr T hi16(T value, bool shift) {
		size_t typeBits = (sizeof(value) > 2) ? 24 : (sizeof(T) * 8);
		auto shiftBits = typeBits - 16;
		auto unshifted = value & (static_cast<T>(0xffff) << shiftBits);
		return shift ? (unshifted >> shiftBits) : unshifted;
	};

	template<typename T>
	static constexpr bool testBit(T value, uint8_t bitIndex) {
		return value & (static_cast<T>(1) << bitIndex);
	};

	template<typename T>
	static constexpr T getBit(uint8_t bitIndex, bool value) {
		if (value) {
			return static_cast<T>(1) << bitIndex;
		} else {
			return 0;
		}
	};

	static std::string valueToHexString(uint32_t value, size_t padToLength = 0, std::string prefix = "") {
		std::stringstream stream;
		stream << prefix << std::hex << std::nouppercase << std::setfill('0') << std::setw(padToLength) << value;
		return stream.str();
	};

	static std::string valueToSignedHexString(int32_t value, size_t padToLength = 0, std::string prefix = "") {
		std::stringstream stream;
		if (value < 0) {
			stream << "-";
			value *= -1;
		}
		stream << prefix << std::hex << std::nouppercase << std::setfill('0') << std::setw(padToLength) << value;
		return stream.str();
	};
	// NOLINTEND(readability-magic-numbers, readability-identifier-length, bugprone-easily-swappable-parameters)

	[[noreturn]] static inline void unreachable() {
#if defined(__GNUC__) // GCC, Clang, ICC
		__builtin_unreachable();
#elif defined(_MSC_VER) // MSVC
		__assume(false);
#else
		throw std::runtime_error("unreachable");
#endif
	};
}; // namespace Blaze
