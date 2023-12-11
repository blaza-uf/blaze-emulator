#include <blaze/DMA.hpp>
#include <blaze/Bus.hpp>
#include <blaze/util.hpp>
#include <blaze/debug.hpp>

#include <cassert>

struct DMAMMIORegister {
	enum IgnoreMe: Blaze::Address {
		DMAP = 0x00,
		BBAD = 0x01,
		A1TL = 0x02,
		A1TH = 0x03,
		A1B  = 0x04,
		DASL = 0x05,
		DASH = 0x06,
		DASB = 0x07,
		A2AL = 0x08,
		A2AH = 0x09,
		NLTR = 0x0a,
	};
};

Blaze::Byte Blaze::DMA::registerSize(Address offset, Byte attemptedAccessSize) {
	return 8;
};

Blaze::Address Blaze::DMA::read(Address offset, Byte bitSize) {
	assert(bitSize == 8);

	if (offset == DMA_SPECIAL_OFFSET_HDMAEN) {
		return _hdmaEnable;
	} else if (offset == DMA_SPECIAL_OFFSET_MDMAEN) {
		return 0;
	} else {
		Channel& channel = _channels[offset / 16];
		Byte channelRegister = offset % 16;

		switch (channelRegister) {
			case DMAMMIORegister::DMAP: return channel.parameters;
			case DMAMMIORegister::BBAD: return channel.peripheralBusAddress;
			case DMAMMIORegister::A1TL: return lo8(channel.cpuBusAddress);
			case DMAMMIORegister::A1TH: return mid8(channel.cpuBusAddress, true);
			case DMAMMIORegister::A1B:  return hi8(channel.cpuBusAddress, true);
			case DMAMMIORegister::DASL: return lo8(channel.byteCountOrIndirectHDMAAddress);
			case DMAMMIORegister::DASH: return hi8(channel.byteCountOrIndirectHDMAAddress, true);
			case DMAMMIORegister::DASB: return channel.indirectHDMABank;
			case DMAMMIORegister::A2AL: return lo8(channel.hdmaTableAddress);
			case DMAMMIORegister::A2AH: return hi8(channel.hdmaTableAddress, true);
			case DMAMMIORegister::NLTR: return channel.hdmaLineCounter;

			default:
				return 0;
		}
	}
};

void Blaze::DMA::write(Address offset, Byte bitSize, Address value) {
	assert(bitSize == 8);

	if (offset == DMA_SPECIAL_OFFSET_HDMAEN) {
		_hdmaEnable = value;

		for (Byte index = 0; index < 8; ++index) {
			if ((_hdmaEnable & (1 << index)) == 0) {
				continue;
			}

			Channel& channel = _channels[index];

			channel.hdmaTableAddress = channel.cpuBusAddress;
			channel.hdmaLineCounter = 0;
		}
	} else if (offset == DMA_SPECIAL_OFFSET_MDMAEN) {
		for (Byte index = 0; index < 8; ++index) {
			if ((value & (1 << index)) == 0) {
				continue;
			}

			Channel& channel = _channels[index];
			Address byteCount = channel.byteCountOrIndirectHDMAAddress; //(channel.byteCount == 0 ? 65536 : channel.byteCount);
			Byte patternIndex = 0;

			auto initialCPUBusAddress = valueToHexString(channel.cpuBusAddress, 6, "$");
			auto initialPeripheralBusAddress = valueToHexString(0x2100 + channel.peripheralBusAddress, 6, "$");

			std::string message = "Initiating transfer from ";

			if (channel.direction() == Direction::AToB) {
				message += initialCPUBusAddress;
			} else {
				message += initialPeripheralBusAddress;
			}

			message += " to ";

			if (channel.direction() == Direction::AToB) {
				message += initialPeripheralBusAddress;
			} else {
				message += initialCPUBusAddress;
			}

			message += " of " + std::to_string(byteCount) + " bytes (pattern = " + std::string(TRANSFER_PATTERN_NAMES[static_cast<Byte>(channel.transferPattern())]);
			message += ", address adjust mode = " + std::string(ADDRESS_ADJUST_MODE_NAMES[static_cast<Byte>(channel.addressAdjustMode())]) + ")";

			Blaze::printLine("dma", message);

			while (byteCount > 0) {
				Address peripheralBusAddress = 0x2100 + channel.peripheralBusAddress;

				switch (channel.transferPattern()) {
					case TransferPattern::SingleByte:
					case TransferPattern::SingleWordRepeated:
					case TransferPattern::AliasedSingleWordRepeated:
						break;

					case TransferPattern::SingleWordSequential:
					case TransferPattern::DoubleWordSequential:
						if (patternIndex == 1 || patternIndex == 3) {
							++peripheralBusAddress;
						}
						break;

					case TransferPattern::DoubleWordRepeated:
					case TransferPattern::AliasedDoubleWordRepeated:
						if (patternIndex == 2 || patternIndex == 3) {
							++peripheralBusAddress;
						}
						break;

					case TransferPattern::QuadByteSequential:
						peripheralBusAddress += patternIndex;
						break;
				}

				if (channel.direction() == Direction::AToB) {
					auto src = _bus->read8(channel.cpuBusAddress);
					_bus->write(peripheralBusAddress, src);
				} else {
					auto src = _bus->read8(peripheralBusAddress);
					_bus->write(channel.cpuBusAddress, src);
				}

				switch (channel.addressAdjustMode()) {
					case AddressAdjustMode::Increment: channel.cpuBusAddress = hi8(channel.cpuBusAddress, false) | lo16(channel.cpuBusAddress + 1); break;
					case AddressAdjustMode::Decrement: channel.cpuBusAddress = hi8(channel.cpuBusAddress, false) | lo16(channel.cpuBusAddress - 1); break;
					case AddressAdjustMode::Fixed: break;
				}

				++patternIndex;
				--byteCount;

				if (patternIndex > 3) {
					patternIndex = 0;
				}
			}

			channel.byteCountOrIndirectHDMAAddress = 0;
		}
	} else {
		Channel& channel = _channels[offset / 16];
		Byte channelRegister = offset % 16;

		switch (channelRegister) {
			case DMAMMIORegister::DMAP: channel.parameters = value; break;
			case DMAMMIORegister::BBAD: channel.peripheralBusAddress = value; break;
			case DMAMMIORegister::A1TL: channel.cpuBusAddress = hi16(channel.cpuBusAddress, false) | value; break;
			case DMAMMIORegister::A1TH: channel.cpuBusAddress = hi8(channel.cpuBusAddress, false) | lo8(channel.cpuBusAddress) | (value << 8); break;
			case DMAMMIORegister::A1B:  channel.cpuBusAddress = lo16(channel.cpuBusAddress) | (value << 16); break;
			case DMAMMIORegister::DASL: channel.byteCountOrIndirectHDMAAddress = hi8(channel.byteCountOrIndirectHDMAAddress, false) | value; break;
			case DMAMMIORegister::DASH: channel.byteCountOrIndirectHDMAAddress = lo8(channel.byteCountOrIndirectHDMAAddress) | (value << 8); break;
			case DMAMMIORegister::DASB: channel.indirectHDMABank = value; break;
			case DMAMMIORegister::A2AL: channel.hdmaTableAddress = hi8(channel.hdmaTableAddress, false) | value; break;
			case DMAMMIORegister::A2AH: channel.hdmaTableAddress = lo8(channel.hdmaTableAddress) | (value << 8); break;
			case DMAMMIORegister::NLTR: channel.hdmaLineCounter = value; break;
		}
	}
};

void Blaze::DMA::reset(Bus* bus) {
	_bus = bus;
};

void Blaze::DMA::performHDMA(Word scanline) {
	for (Byte index = 0; index < 8; ++index) {
		if ((_hdmaEnable & (1 << index)) == 0) {
			continue;
		}

		Channel& channel = _channels[index];

		auto byteCount = transferPatternHDMABytes(channel.transferPattern());
		bool doWrite = false;

		if ((channel.hdmaLineCounter & 0x7f) == 0) {
			// load the next table entry
			channel.hdmaLineCounter = _bus->read8(channel.hdmaTableAddress);

			if (channel.indirect()) {
				channel.byteCountOrIndirectHDMAAddress = _bus->read16(channel.hdmaTableAddress + 1);
				channel.hdmaTableAddress = hi8(channel.hdmaTableAddress, false) | lo16(channel.hdmaTableAddress + 3);
			} else {
				channel.hdmaTableAddress = hi8(channel.hdmaTableAddress, false) | lo16(channel.hdmaTableAddress + 1);
			}

			// we always write at least one pattern when loading a new entry
			doWrite = true;
		} else if ((channel.hdmaLineCounter & (1 << 7)) != 0) {
			// repeat mode and *not* loading a new entry
			doWrite = true;
		}

		if (doWrite) {
			Byte patternIndex = 0;
			auto writeCount = 0;

			Address cpuBusAddress = 0;

			if (channel.indirect()) {
				cpuBusAddress = channel.indirectHDMAAddress();
			} else {
				cpuBusAddress = channel.hdmaTableAddress;
			}

			auto initialCPUBusAddress = valueToHexString(cpuBusAddress, 6, "$");
			auto initialPeripheralBusAddress = valueToHexString(0x2100 + channel.peripheralBusAddress, 6, "$");

			std::string message = "Initiating HDMA transfer from ";
			message += initialCPUBusAddress;
			message += " to ";
			message += initialPeripheralBusAddress;
			message += " of " + std::to_string(byteCount) + " bytes (pattern = " + std::string(TRANSFER_PATTERN_NAMES[static_cast<Byte>(channel.transferPattern())]);
			message += ", address adjust mode = " + std::string(ADDRESS_ADJUST_MODE_NAMES[static_cast<Byte>(channel.addressAdjustMode())]) + ")";

			//Blaze::printLine("dma", message);

			while (writeCount > 0) {
				Address peripheralBusAddress = 0x2100 + channel.peripheralBusAddress;

				switch (channel.transferPattern()) {
					case TransferPattern::SingleByte:
					case TransferPattern::SingleWordRepeated:
					case TransferPattern::AliasedSingleWordRepeated:
						break;

					case TransferPattern::SingleWordSequential:
					case TransferPattern::DoubleWordSequential:
						if (patternIndex == 1 || patternIndex == 3) {
							++peripheralBusAddress;
						}
						break;

					case TransferPattern::DoubleWordRepeated:
					case TransferPattern::AliasedDoubleWordRepeated:
						if (patternIndex == 2 || patternIndex == 3) {
							++peripheralBusAddress;
						}
						break;

					case TransferPattern::QuadByteSequential:
						peripheralBusAddress += patternIndex;
						break;
				}

				auto src = _bus->read8(cpuBusAddress);
				_bus->write(peripheralBusAddress, src);

				++patternIndex;
				--writeCount;
				++cpuBusAddress;

				if (patternIndex > 3) {
					patternIndex = 0;
				}
			}

			if (channel.indirect()) {
				channel.byteCountOrIndirectHDMAAddress += byteCount;
			} else {
				channel.hdmaTableAddress = hi8(channel.hdmaTableAddress, false) | lo16(channel.hdmaTableAddress + byteCount);
			}
		}

		channel.hdmaLineCounter = (channel.hdmaLineCounter & (1 << 7)) | ((channel.hdmaLineCounter - 1) & 0x7f);

#if 0
		if ((channel.hdmaLineCounter & 0x7f) == 0) {
			// disable this channel
			_hdmaEnable &= ~(1 << index);
		}
#endif
	}
};
