#include <blaze/DMA.hpp>
#include <blaze/Bus.hpp>

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
			case DMAMMIORegister::DASL: return lo8(channel.byteCount);
			case DMAMMIORegister::DASH: return hi8(channel.byteCount, true);
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
		// TODO: HDMA
	} else if (offset == DMA_SPECIAL_OFFSET_MDMAEN) {
		for (Byte index = 0; index < 8; ++index) {
			if ((value & (1 << index)) == 0) {
				continue;
			}

			Channel& channel = _channels[index];
			Address byteCount = (channel.byteCount == 0 ? 65536 : channel.byteCount);
			Byte patternIndex = 0;

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

				if (patternIndex > 3) {
					patternIndex = 0;
				}
			}

			channel.byteCount = 0;
		}
	} else {
		Channel& channel = _channels[offset / 16];
		Byte channelRegister = offset % 16;

		switch (channelRegister) {
			case DMAMMIORegister::DMAP: channel.parameters = value;
			case DMAMMIORegister::BBAD: channel.peripheralBusAddress = value;
			case DMAMMIORegister::A1TL: channel.cpuBusAddress = hi16(channel.cpuBusAddress, false) | value;
			case DMAMMIORegister::A1TH: channel.cpuBusAddress = hi8(channel.cpuBusAddress, false) | lo8(channel.cpuBusAddress) | (value << 8);
			case DMAMMIORegister::A1B:  channel.cpuBusAddress = lo16(channel.cpuBusAddress) | (value << 16);
			case DMAMMIORegister::DASL: channel.byteCount = hi8(channel.byteCount, false) | value;
			case DMAMMIORegister::DASH: channel.byteCount = lo8(channel.byteCount) | (value << 8);
			case DMAMMIORegister::DASB: channel.indirectHDMABank = value;
			case DMAMMIORegister::A2AL: channel.hdmaTableAddress = hi8(channel.hdmaTableAddress, false) | value;
			case DMAMMIORegister::A2AH: channel.hdmaTableAddress = lo8(channel.hdmaTableAddress) | (value << 8);
			case DMAMMIORegister::NLTR: channel.hdmaLineCounter = value;
		}
	}
};

void Blaze::DMA::reset(Bus* bus) {
	_bus = bus;
};
