#pragma once

#include <blaze/MMIO.hpp>
#include <blaze/util.hpp>

#include <array>

namespace Blaze {
	struct Bus;

	// these are special offsets that have nothing to do with their actual addresses
	// because these two registers don't lie within the memory region used for all the
	// other DMA registers.
	static constexpr Address DMA_SPECIAL_OFFSET_MDMAEN = 0xffff00;
	static constexpr Address DMA_SPECIAL_OFFSET_HDMAEN = 0xffff80;

	class DMA: public MMIODevice {
	private:
		enum class TransferPattern: Byte {
			SingleByte = 0,
			SingleWordSequential = 1,
			SingleWordRepeated = 2,
			DoubleWordRepeated = 3,
			QuadByteSequential = 4,
			DoubleWordSequential = 5,
			AliasedSingleWordRepeated = 6,
			AliasedDoubleWordRepeated = 7,

			LAST = AliasedDoubleWordRepeated,
		};

		static constexpr std::array<const char*, static_cast<Byte>(TransferPattern::LAST) + 1> TRANSFER_PATTERN_NAMES = {
			"SingleByte",
			"SingleWordSequential",
			"SingleWordRepeated",
			"DoubleWordRepeated",
			"QuadByteSequential",
			"DoubleWordSequential",
			"AliasedSingleWordRepeated",
			"AliasedDoubleWordRepeated",
		};

		static constexpr Byte transferPatternHDMABytes(TransferPattern pattern) {
			switch (pattern) {
				case TransferPattern::SingleByte:
					return 1;
				case TransferPattern::SingleWordSequential:
				case TransferPattern::SingleWordRepeated:
				case TransferPattern::AliasedSingleWordRepeated:
					return 2;
				case TransferPattern::DoubleWordRepeated:
				case TransferPattern::QuadByteSequential:
				case TransferPattern::DoubleWordSequential:
				case TransferPattern::AliasedDoubleWordRepeated:
					return 4;
				default:
					return 0;
			}
		};

		enum class Direction: bool {
			AToB = false,
			BToA = true,
		};

		enum class AddressAdjustMode: Byte {
			Increment = 0,
			Decrement = 1,
			Fixed = 2,

			LAST = Fixed,
		};

		static constexpr std::array<const char*, static_cast<Byte>(AddressAdjustMode::LAST) + 1> ADDRESS_ADJUST_MODE_NAMES = {
			"Increment",
			"Decrement",
			"Fixed",
		};

		struct Channel {
			Byte parameters = 0xff;
			Byte peripheralBusAddress = 0xff;
			Address cpuBusAddress = 0xffffff;
			Word byteCountOrIndirectHDMAAddress = 0xffff;
			Byte indirectHDMABank = 0xff;
			Address hdmaTableAddress = 0xffffff;
			Byte hdmaLineCounter = 0xff;

			inline TransferPattern transferPattern() const {
				return static_cast<TransferPattern>(parameters & 7);
			};

			inline AddressAdjustMode addressAdjustMode() const {
				switch ((parameters >> 3) & 3) {
					case 0: return AddressAdjustMode::Increment;
					case 2: return AddressAdjustMode::Decrement;
					case 1:
					case 3: return AddressAdjustMode::Fixed;
					default: unreachable();
				}
			};

			inline Direction direction() const {
				return static_cast<Direction>((parameters & (1 << 7)) != 0);
			};

			inline bool indirect() const {
				return (parameters & (1 << 6)) != 0;
			};

			inline Address indirectHDMAAddress() const {
				return concat24(indirectHDMABank, byteCountOrIndirectHDMAAddress);
			};
		};

		Bus* _bus = nullptr;
		Byte _hdmaEnable = 0;
		std::array<Channel, 8> _channels;

	public:
		Byte registerSize(Address offset, Byte attemptedAccessSize) override;
		Address read(Address offset, Byte bitSize) override;
		void write(Address offset, Byte bitSize, Address value) override;

		void reset(Bus* bus) override;

		void performHDMA(Word scanline);
	};
};
