#pragma once

#include <blaze/MMIO.hpp>

#include <array>

namespace Blaze {
	class APU: public MMIODevice {
	private:
		enum class State: Byte {
			// waiting for $CC on port 0
			WaitForSignal,

			// read port 1 to determine whether to execute code or begin transferring data
			Begin,

			// wait for port 0 to read 0
			TransferLoop,

			// wait until port 0 reads greater than or equal to the internal counter
			WaitForCounter,

			// execute code
			Execute,
		};

		Bus* _bus = nullptr;
		std::array<Byte, 4> _portsFromCPU;
		std::array<Byte, 4> _portsToCPU;
		State _state = State::WaitForSignal;
		Word _address = 0;
		Byte _counter = 0;

	public:
		Byte registerSize(Address offset, Byte attemptedAccessSize) override;
		Address read(Address offset, Byte bitSize) override;
		void write(Address offset, Byte bitSize, Address value) override;

		void reset(Bus* bus) override;
	};
};
