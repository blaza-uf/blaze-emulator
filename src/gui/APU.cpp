#include <blaze/APU.hpp>
#include <blaze/util.hpp>
#include <blaze/debug.hpp>

#include <cassert>
#include <algorithm>

Blaze::Byte Blaze::APU::registerSize(Address offset, Byte attemptedAccessSize) {
	return 8;
};

Blaze::Address Blaze::APU::read(Address offset, Byte bitSize) {
	assert(bitSize == 8);
	return _portsToCPU[offset];
};

void Blaze::APU::write(Address offset, Byte bitSize, Address value) {
	assert(bitSize == 8);

	_portsFromCPU[offset] = value;

	switch (_state) {
		case State::WaitForSignal:
			if (_portsFromCPU[0] == 0xcc) {
				_state = State::Begin;

				Blaze::printLine("apu", "Received signal to continue");

				_address = concat16(_portsFromCPU[3], _portsFromCPU[2]);
				_portsToCPU[0] = _portsFromCPU[0];
			}
			break;

		case State::Begin:
			if (_portsFromCPU[1] == 0) {
				_state = State::Execute;
				Blaze::printLine("apu", "Beginning code execution");
			} else {
				_state = State::TransferLoop;
				Blaze::printLine("apu", "Beginning transfer loop");
			}
			break;

		case State::TransferLoop:
			if (_portsFromCPU[0] == 0) {
				_state = State::WaitForCounter;

				_counter = 0;

				// read from port 1 and write to destination address...
				// ...but we don't actually do that; we just discard the data.

				//Blaze::printLine("apu", "Transfer loop read " + valueToHexString(_portsFromCPU[1], 2, "$"));

				// write the value of port 0 back to port 0 to acknowledge the data
				_portsToCPU[0] = _portsFromCPU[0];

				// increment the address and counter
				++_address;
				++_counter;
			}
			break;

		case State::WaitForCounter:
			if (_portsFromCPU[0] == _counter) {
				// read from port 1 and write to destination address...
				// ...but we don't actually do that; we just discard the data.

				//Blaze::printLine("apu", "Transfer loop read " + valueToHexString(_portsFromCPU[1], 2, "$"));

				// write the value of port 0 back to port 0 to acknowledge the data
				_portsToCPU[0] = _portsFromCPU[0];

				// increment the address and counter
				++_address;
				++_counter;
			} else if (_portsFromCPU[0] > _counter) {
				_state = State::Begin;

				Blaze::printLine("apu", "Ending transfer loop");

				// write the value of port 0 back to port 0 to acknowledge the loop end
				_portsToCPU[0] = _portsFromCPU[0];

				_address = concat16(_portsFromCPU[3], _portsFromCPU[2]);
				_portsToCPU[0] = _portsFromCPU[0];
			}
			break;

		case State::Execute:
			break;
	}
};

void Blaze::APU::reset(Bus* bus) {
	_bus = bus;
	_state = State::WaitForSignal;
	std::fill(_portsFromCPU.begin(), _portsFromCPU.end(), 0);
	std::fill(_portsToCPU.begin(), _portsToCPU.end(), 0);

	_portsToCPU[0] = 0xaa;
	_portsToCPU[1] = 0xbb;
};
