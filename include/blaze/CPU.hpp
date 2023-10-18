#pragma once

#include <blaze/MemTypes.hpp>
#include <blaze/MemRam.hpp>
#include <limits>
#include <array>
#include <unordered_map>

namespace Blaze {
	using ClockTicks = uint32_t;
	using Cycles = uint32_t;

	// Avoid circular inclusions by declaring Bus
	struct Bus;

	struct CPU {
		// TODO: Link to the system bus

		//
		// The various addressing modes support by the 65C816.
		//
		// I've added comments describing exactly how their addresses are calculated.
		// Note that in these notes, `inst` is an array pointing to the bytes (`u8`) of the
		// instruction. This array is zero-indexed, with `inst[0]` being the opcode of the
		// instruction. For simplicity, when necessary, opcodes will be compared by name
		// in the notes (rather than by numeric values). For example, `inst[0] == JML` is
		// true when the instruction opcode is for a `JML` instruction.
		//
		// Whenever addition is present, assume that it takes place in the largest bitwidth
		// of the operands.
		//
		// Note that the program counter relative modes below use the program counter AFTER
		// it has been updated to point to the next instruction.
		//
		enum class AddressingMode: uint8_t {
			Absolute,                     // addr = u24:concat(DBR, inst[2], inst[1])
			AbsoluteIndexedIndirect,      // addr = u16:load(u8:0, u16:concat(inst[2], inst[1]) + X)
			AbsoluteIndexedX,             // addr = u24:concat(DBR, u16:concat(inst[2], inst[1]) + X)
			AbsoluteIndexedY,             // addr = u24:concat(DBR, u16:concat(inst[2], inst[1]) + Y)
			AbsoluteIndirect,             // addr = IF(inst[0] == JML, u24, u16):load(u8:0, u16:concat(inst[2], inst[1]))
			AbsoluteLongIndexedX,         // addr = u24:concat(inst[3], inst[2], inst[1]) + X
			AbsoluteLong,                 // addr = u24:concat(inst[3], inst[2], inst[1])
			Accumulator,                  // not applicable; indicates data is being transferred to/from the accumulator register
			BlockMove,                    // srcaddr = u24:concat(inst[2], X); dstaddr = u24:concat(inst[1], Y)
			DirectIndexedIndirect,        // addr = u24:concat(DBR, u16:load(u8:0, DR + X + inst[1]))
			DirectIndexedX,               // addr = u24:concat(u8:0, DR + X + inst[1])
			DirectIndexedY,               // addr = u24:concat(u8:0, DR + Y + inst[1])
			DirectIndirectIndexed,        // addr = u24:concat(DBR, u16:load(u8:0, DR + inst[1])) + Y
			DirectIndirectLongIndexed,    // addr = u24:load(u8:0, DR + inst[1]) + Y
			DirectIndirectLong,           // addr = u24:load(u8:0, DR + inst[1])
			DirectIndirect,               // addr = u24:concat(DBR, u16:load(u8:0, DR + inst[1]))
			Direct,                       // addr = u24:concat(u8:0, DR + inst[1])
			Immediate,                    // not applicable; indicates data is being read from `inst[1]` (and `inst[2]` when in 16-bit mode)
			Implied,                      // not applicable; indicates the the operand is implied by the opcode
			ProgramCounterRelativeLong,   // addr = u16:(PC + i16:concat(inst[2], inst[1]))
			ProgramCounterRelative,       // addr = u16:(PC + i8:inst[1])
			Stack,                        // not applicable; indicates data is being transferred to/from the stack (via the current address in the stack pointer register)
			StackRelative,                // addr = u24:concat(u8:0, SP + inst[1])
			StackRelativeIndirectIndexed, // addr = u24:concat(DBR, u16:load(u8:0, SP + inst[1])) + Y

			Last = StackRelativeIndirectIndexed,
			INVALID = std::numeric_limits<uint8_t>::max(),
		};

		static constexpr Byte instructionSizeWithAddressingMode(AddressingMode mode) {
			switch (mode) {
				case AddressingMode::Accumulator:
				case AddressingMode::Implied:
					return 1;

				case AddressingMode::DirectIndexedIndirect:
				case AddressingMode::DirectIndexedX:
				case AddressingMode::DirectIndexedY:
				case AddressingMode::DirectIndirectIndexed:
				case AddressingMode::DirectIndirectLongIndexed:
				case AddressingMode::DirectIndirectLong:
				case AddressingMode::DirectIndirect:
				case AddressingMode::Direct:
				case AddressingMode::ProgramCounterRelative:
				case AddressingMode::StackRelative:
				case AddressingMode::StackRelativeIndirectIndexed:
					return 2;

				case AddressingMode::Absolute:
				case AddressingMode::AbsoluteIndexedIndirect:
				case AddressingMode::AbsoluteIndexedX:
				case AddressingMode::AbsoluteIndexedY:
				case AddressingMode::AbsoluteIndirect:
				case AddressingMode::BlockMove:
				case AddressingMode::ProgramCounterRelativeLong:
					return 3;

				case AddressingMode::AbsoluteLongIndexedX:
				case AddressingMode::AbsoluteLong:
					return 4;

				case AddressingMode::Immediate: // only known depending on CPU flags
				case AddressingMode::Stack:     // only known per-instruction
				default:
					return 0;
			}
		};

		// 6502 - Group 1 (cc=01)
		enum class Group1Opcode: uint8_t {
			ORA = 0,
			AND,
			EOR,
			ADC,
			STA,
			LDA,
			CMP,
			SBC,

			Last = SBC,
		};

		static constexpr std::array<AddressingMode, 8> GROUP1_6502_ADDRESS_MODE_MAP = {
			AddressingMode::DirectIndexedIndirect,
			AddressingMode::Direct,
			AddressingMode::Immediate,
			AddressingMode::Absolute,
			AddressingMode::DirectIndirectIndexed,
			AddressingMode::DirectIndexedX,
			AddressingMode::AbsoluteIndexedY,
			AddressingMode::AbsoluteIndexedX,
		};

		static constexpr std::array<AddressingMode, 8> GROUP1_65C816_ADDRESS_MODE_MAP = {
			AddressingMode::StackRelative,
			AddressingMode::DirectIndirectLong,
			AddressingMode::INVALID,
			AddressingMode::AbsoluteLong,
			AddressingMode::StackRelativeIndirectIndexed,
			AddressingMode::DirectIndirectLongIndexed,
			AddressingMode::INVALID,
			AddressingMode::AbsoluteLongIndexedX,
		};

		// 6502 - Group 2 (cc=10)
		enum class Group2Opcode: uint8_t {
			ASL = 0,
			ROL,
			LSR,
			ROR,
			STX,
			LDX,
			DEC,
			INC,

			Last = INC,
		};

		static constexpr std::array<AddressingMode, 8> GROUP2_ADDRESS_MODE_MAP = {
			AddressingMode::Immediate,
			AddressingMode::Direct,
			AddressingMode::Accumulator,
			AddressingMode::Absolute,
			AddressingMode::INVALID,
			AddressingMode::DirectIndexedX,
			AddressingMode::INVALID,
			AddressingMode::AbsoluteIndexedX,
		};

		// Group 2 instructions with this address mode are actually Group 1 instructions with Direct Indirect addressing (introduced on the 65C02)
		static constexpr Byte GROUP2_65C02_ADDRESS_MODE = 4;

		// 6502 - Group 3 (cc=00)
		enum class Group3Opcode: uint8_t {
			TSB = 0,
			BIT,
			JMP,
			JMPIndirect,
			STY,
			LDY,
			CPY,
			CPX,

			Last = CPX,
		};

		static constexpr std::array<AddressingMode, 8> GROUP3_ADDRESS_MODE_MAP = {
			AddressingMode::Immediate,
			AddressingMode::Direct,
			AddressingMode::INVALID,
			AddressingMode::Absolute,
			AddressingMode::INVALID,
			AddressingMode::DirectIndexedX,
			AddressingMode::INVALID,
			AddressingMode::AbsoluteIndexedX,
		};

		enum class ConditionCode: uint8_t {
			Negative = 0,
			Overflow,
			Carry,
			Zero,

			NONE = 0xff,
		};

		// Group 3 instructions with this address mode are actually branch instructions with a condition
		static constexpr Byte GROUP3_CONDITION_ADDRESS_MODE = 4;

		enum class Opcode: Byte {
			BRK,
			BRL,
			CLC,
			CLD,
			CLI,
			CLV,
			COP,
			DEX,
			DEY,
			INX,
			INY,
			JML,
			JSL,
			MVN,
			MVP,
			NOP,
			PEA,
			PEI,
			PER,
			PHA,
			PHB,
			PHD,
			PHK,
			PHP,
			PHX,
			PHY,
			PLA,
			PLB,
			PLD,
			PLP,
			PLX,
			PLY,
			REP,
			RTI,
			RTL,
			RTS,
			SEC,
			SED,
			SEI,
			SEP,
			STP,
			TAX,
			TAY,
			TCD,
			TCS,
			TDC,
			TSC,
			TSX,
			TXA,
			TXS,
			TXY,
			TYA,
			TYX,
			WAI,
			WDM,
			XBA,
			XCE,

			ADC,
			AND,
			ASL,
			BIT,
			CMP,
			CPX,
			CPY,
			DEC,
			EOR,
			INC,
			JMP,
			JSR,
			LDA,
			LDX,
			LDY,
			LSR,
			ORA,
			ROL,
			ROR,
			SBC,
			STA,
			STX,
			STY,
			STZ,
			TRB,
			TSB,

			BRA,

			INVALID = std::numeric_limits<Byte>::max(),
		};

		struct Instruction {
			Opcode opcode = Opcode::INVALID;
			Byte size = 0;

			// Only valid for some opcodes
			AddressingMode addressingMode = AddressingMode::INVALID;

			// Only for `BRA`
			ConditionCode condition = ConditionCode::NONE;

			// Only for `BRA`
			bool passConditionIfBitSet = false;

			// For some instructions, this is not known ahead of time (before execution).
			// For such instructions, leave this as 0.
			Cycles cycles = 0;

			// This default constructor produces an invalid instruction (i.e. one for which the `valid` method returns `false`).
			constexpr Instruction() = default;

			constexpr Instruction(Opcode opcode, Byte size, Cycles cycles, AddressingMode addressingMode = AddressingMode::INVALID):
				opcode(opcode),
				size(size),
				addressingMode(addressingMode),
				cycles(cycles)
				{};

			constexpr Instruction(Opcode opcode, Byte size, Cycles cycles, ConditionCode condition, bool passConditionIfBitSet):
				opcode(opcode),
				size(size),
				condition(condition),
				passConditionIfBitSet(passConditionIfBitSet),
				cycles(cycles)
				{};

			inline bool valid() const {
				return opcode != Opcode::INVALID && size > 0;
			};

			inline bool hasFixedCycleCount() const {
				return cycles > 0;
			};

			inline operator bool() const {
				return valid();
			};
		};

		static const std::unordered_map<Byte, Instruction> INSTRUCTIONS_WITH_NO_PATTERN;

		enum flags: Byte {
			// Process Status Flags
			c = (1 << 0), // carry
			z = (1 << 1), // zero
			i = (1 << 2), // interrupt disable
			d = (1 << 3), // decimal
			x = (1 << 4), // index register width
			b = (1 << 4), // break
			m = (1 << 5), // accumulator & memory width
			v = (1 << 6), // overflow
			n = (1 << 7), // negative
		} ; flags f;

		Byte e = 1; //emulation mode. separate from p register flags

		Word A; // accumulator
		Word DR; // direct
		Word PC; // program counter
		Word X, Y; // index registers
		Word SP; // stack pointer
		Byte DBR; // data bank
		Byte PBR; // program bank
		Byte P; // process status
		MemRam* _memory; // TODO: Replace direct memory access with Bus r/w

		// System Bus
		Bus *bus = nullptr;

		Byte load8(Address address) const;
		Byte load8(Byte bank, Word addressLow) const;
		Word load16(Address address) const;
		Word load16(Byte bank, Word addressLow) const;
		Address load24(Address address) const;
		Address load24(Byte bank, Word addressLow) const;

		void store8(Address address, Byte value);
		void store8(Byte bank, Word addressLow, Byte value);
		void store16(Address address, Word value);
		void store16(Byte bank, Word addressLow, Word value);
		void store24(Address address, Address value);
		void store24(Byte bank, Word addressLow, Address value);

		// this function is meant to be called by instruction execution functions to obtain the address
		// of the operand with the given addressing mode.
		Address decodeAddress(AddressingMode addressingMode) const;

		// this function is meant to be used by simple instructions that only need to load data from the
		// memory operands (which is true for most instructions). if you need to both read from and write to
		// a memory operand, you should use `decodeAddress` + `load16` instead.
		Word loadOperand(AddressingMode addressingMode) const;

		// decodes the current instruction based on the given opcode, returning the decoded instruction information
		Instruction decodeInstruction(Byte opcode) const;

		// executes the current (pre-decoded) instruction with the given information
		Cycles executeInstruction(const Instruction& info);

		Cycles invalidInstruction();

		const Byte* currentInstruction() const;

		Cycles executeBRK();
		Cycles executeBRL();
		Cycles executeCLC();
		Cycles executeCLD();
		Cycles executeCLI();
		Cycles executeCLV();
		Cycles executeCOP();
		Cycles executeDEX();
		Cycles executeDEY();
		Cycles executeINX();
		Cycles executeINY();
		Cycles executeJML();
		Cycles executeJSL();
		Cycles executeMVN();
		Cycles executeMVP();
		Cycles executeNOP();
		Cycles executePEA();
		Cycles executePEI();
		Cycles executePER();
		Cycles executePHA();
		Cycles executePHB();
		Cycles executePHD();
		Cycles executePHK();
		Cycles executePHP();
		Cycles executePHX();
		Cycles executePHY();
		Cycles executePLA();
		Cycles executePLB();
		Cycles executePLD();
		Cycles executePLP();
		Cycles executePLX();
		Cycles executePLY();
		Cycles executeREP();
		Cycles executeRTI();
		Cycles executeRTL();
		Cycles executeRTS();
		Cycles executeSEC();
		Cycles executeSED();
		Cycles executeSEI();
		Cycles executeSEP();
		Cycles executeSTP();
		Cycles executeTAX();
		Cycles executeTAY();
		Cycles executeTCD();
		Cycles executeTCS();
		Cycles executeTDC();
		Cycles executeTSC();
		Cycles executeTSX();
		Cycles executeTXA();
		Cycles executeTXS();
		Cycles executeTXY();
		Cycles executeTYA();
		Cycles executeTYX();
		Cycles executeWAI();
		Cycles executeWDM();
		Cycles executeXBA();
		Cycles executeXCE();

		Cycles executeADC(AddressingMode mode);
		Cycles executeAND(AddressingMode mode);
		Cycles executeASL(AddressingMode mode);
		Cycles executeBIT(AddressingMode mode);
		Cycles executeCMP(AddressingMode mode);
		Cycles executeCPX(AddressingMode mode);
		Cycles executeCPY(AddressingMode mode);
		Cycles executeDEC(AddressingMode mode);
		Cycles executeEOR(AddressingMode mode);
		Cycles executeINC(AddressingMode mode);
		Cycles executeJMP(AddressingMode mode);
		Cycles executeJSR(AddressingMode mode);
		Cycles executeLDA(AddressingMode mode);
		Cycles executeLDX(AddressingMode mode);
		Cycles executeLDY(AddressingMode mode);
		Cycles executeLSR(AddressingMode mode);
		Cycles executeORA(AddressingMode mode);
		Cycles executeROL(AddressingMode mode);
		Cycles executeROR(AddressingMode mode);
		Cycles executeSBC(AddressingMode mode);
		Cycles executeSTA(AddressingMode mode);
		Cycles executeSTX(AddressingMode mode);
		Cycles executeSTY(AddressingMode mode);
		Cycles executeSTZ(AddressingMode mode);
		Cycles executeTRB(AddressingMode mode);
		Cycles executeTSB(AddressingMode mode);

		Cycles executeBRA(ConditionCode condition, bool passConditionIfBitSet);

		void reset(MemRam &memory);      		// Reset CPU internal state
		void execute(); 		// Execute the current instruction
		void clock();                    		// CPU driver
		Byte read(Address addr);				// Read from the Bus
		void write(Address addr, Byte data);	// Write to the Bus

		// Interrupt Handling
		Cycles cyclesCountDown = 0;					// Counts how many cycles the instruction has remaining
		ClockTicks clockCount = 0;					// A global accumulation of the number of clocks
		Address addrAbs = 0x00000000;				// The address from last visit
		void irq();
		void nmi();

		bool getFlag(flags f) const;
		void setFlag(flags f, bool s);

		Byte getCarry() const {
			return getFlag(flags::c) ? 1 : 0;
		};

		void setZeroNegFlags(Word a_x_y, bool isAccumulator);
		// this is only used for ADC and SBC
		void setOverflowFlag(Word leftOperand, Word rightOperand, Word result);

		// just a convenience method to make it more clear what we're checking for
		bool memoryAndAccumulatorAre8Bit() const {
			return getFlag(flags::m);
		};

		bool indexRegistersAre8Bit() const {
			return getFlag(flags::x);
		};
	};
} // namespace Blaze
