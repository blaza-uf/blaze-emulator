#pragma once

#include "blaze/CPU.hpp"

namespace Blaze
{
	struct Bus
	{
		//=== Devices connected to the bus ===
		CPU cpu;
		MemRam ram;

		//=== Constructor & Destructor ===
		Bus();
		~Bus();

		//=== Bus Fucntionality ===
		void write(Address addr, Byte data);
		Byte* read(Address addr);
	};
}