#pragma once

#include <blaze/Bus.hpp>
#include <utility>

#include <catch2/catch_test_macros.hpp>

namespace Blaze::Testing {
	// a testing mock replacement for the Bus struct to test memory access
	// at arbitrary indices
	class Bus: public Blaze::BusInterface {
	public:
		using ReadHook = std::function<Address(Address address, Byte bitSize)>;
		using WriteHook = std::function<void(Address address, Address value, Byte bitSize)>;

	protected:
		ReadHook _read = nullptr;
		WriteHook _write = nullptr;

		Bus() {};

	public:
		Bus(ReadHook read, WriteHook write):
			_read(std::move(read)),
			_write(std::move(write))
			{};

		void write(Address addr, Byte data) override {
			_write(addr, data, 8);
		};
		void write(Address addr, Word data) override {
			_write(addr, data, 16);
		};
		void write(Address addr, Address data) override {
			_write(addr, data, 24);
		};
		Byte read8(Address addr) override {
			return _read(addr, 8);
		};
		Word read16(Address addr) override {
			return _read(addr, 16);
		};
		Address read24(Address addr) override {
			return _read(addr, 24);
		};
	};

	struct BusAccess {
		bool isWrite;
		Address address;
		Byte bitSize;
		Address value;

		BusAccess(bool isWrite, Address address, Byte bitSize, Address value):
			isWrite(isWrite),
			address(address),
			bitSize(bitSize),
			value(value)
			{};
	};

	class PreconfiguredBus: public Blaze::BusInterface {
	private:
		bool _ordered;
		std::vector<BusAccess> _accesses;
		size_t _index = 0;

		Address testRead(Address address, Byte bitSize) {
			if (_ordered) {
				if (_index >= _accesses.size()) {
					FAIL("All bus accesses exhausted");
					return 0;
				}

				const auto& access = _accesses[_index++];

				REQUIRE_FALSE(access.isWrite);
				REQUIRE(access.address == address);
				REQUIRE(access.bitSize == bitSize);

				return access.value;
			} else {
				for (const auto& access: _accesses) {
					if (!access.isWrite && access.address == address && access.bitSize == bitSize) {
						return access.value;
					}
				}

				FAIL("No corresponding access found for read");
				return 0;
			}
		};

		void testWrite(Address address, Address value, Byte bitSize) {
			if (_ordered) {
				if (_index >= _accesses.size()) {
					FAIL("All bus accesses exhausted");
					return;
				}

				const auto& access = _accesses[_index++];

				REQUIRE(access.isWrite);
				REQUIRE(access.address == address);
				REQUIRE(access.bitSize == bitSize);
				REQUIRE(access.value == value);
			} else {
				for (const auto& access: _accesses) {
					if (access.isWrite && access.address == address && access.bitSize == bitSize && access.value == value) {
						return;
					}
				}

				FAIL("No corresponding access found for write");
			}
		};

	public:
		PreconfiguredBus(bool ordered, std::vector<BusAccess> accesses):
			_ordered(ordered),
			_accesses(std::move(accesses))
			{};

		void write(Address addr, Byte data) override {
			testWrite(addr, data, 8);
		};
		void write(Address addr, Word data) override {
			testWrite(addr, data, 16);
		};
		void write(Address addr, Address data) override {
			testWrite(addr, data, 24);
		};
		Byte read8(Address addr) override {
			return testRead(addr, 8);
		};
		Word read16(Address addr) override {
			return testRead(addr, 16);
		};
		Address read24(Address addr) override {
			return testRead(addr, 24);
		};

		void finalize() {
			if (_ordered && _index != _accesses.size()) {
				FAIL("Not all expected bus accesses were completed before finalization");
			}
		};
	};
};
