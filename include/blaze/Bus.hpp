#pragma once

#include <blaze/CPU.hpp>
#include <blaze/MemRam.hpp>
#include <blaze/ROM.hpp>
#include <blaze/MMIO.hpp>

namespace Blaze
{
	struct Bus
	{
		//=== Devices connected to the bus ===
		CPU cpu;
		MemRam ram;
		ROM rom;

		//=== Constructor & Destructor ===
		Bus();

		//=== Bus Functionality ===
		void write(Address addr, Byte data); 	// write8
		void write(Address addr, Word data); 	// write16
		void write(Address addr, Address data); // write 24
		Byte read8(Address addr);
		Word read16(Address addr);
		Address read24(Address addr);

		void reset();

	private:
		void findDeviceAndOffset(Address address, MMIODevice*& outDevice, Address& outOffset);
	};
}
