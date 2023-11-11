#pragma once

#include <blaze/MemTypes.hpp>

#include <string>
#include <array>
#include <vector>

namespace Blaze {
	class ROM {
	public:
		enum class Type: Byte {
			INVALID,
			LoROM,
			HiROM,
			ExLoROM,
			ExHiROM,
		};

		enum class CartridgeType: Byte {
			ROMOnly = 0,
			ROM_RAM = 1,
			ROM_RAM_Battery = 2,
			ROM_SA1 = 0x33,
			ROM_SA1_RAM = 0x34,
			ROM_SA1_RAM_Battery = 0x35,
		};

		enum class DestinationCode: Byte {
			Japan = 0,
			USA = 1,
			Europe = 2,
		};

		struct HeaderFieldOffset {
			enum IgnoreMe: Byte {
				MakerCode = 0x00,
				GameCode = 0x02,
				ExpansionROMSize = 0x0d,
				SpecialVersion = 0x0e,
				CartridgeTypeSubNumber = 0x0f,
				GameTitle = 0x10,
				MappingType = 0x25,
				CartridgeType = 0x26,
				Size = 0x27,
				RAMSize = 0x28,
				DestinationCode = 0x29,
				FixedValue = 0x2a,
				Version = 0x2b,
				ChecksumComplement = 0x2c,
				Checksum = 0x2e,
			};
		};

	private:
		std::vector<Byte> _memory;
		Type _type = Type::INVALID;

		size_t headerOffset() const;

	public:
		Type type() const;
		size_t byteSize() const;
		std::string name() const;

		void load(const std::string& path);
	};
} // namespace Blaze
