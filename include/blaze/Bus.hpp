#pragma once

#include"CPU.hpp"

class Bus
{	
public:
	// Devices connected to the bus
	// TODO: Connect cpu and RAM
	
	//=== Constructor & Destructor ===
	Bus();
	~Bus();

	//=== Bus Fucntionality ===
	void write(Address addr, Byte data);
	Byte read(Word addr);
};