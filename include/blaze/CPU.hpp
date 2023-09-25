#pragma once

#include <blaze/MemTypes.hpp>
#include <blaze/MemRam.hpp>
#include <limits>
#include <array>
#include <unordered_map>

namespace Blaze {
	using ClockTicks = uint32_t;
	using Cycles = uint32_t;

	class CPU {
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
			AddressingMode::INVALID,
			AddressingMode::DirectIndexedX,
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
			AddressingMode::INVALID,
			AddressingMode::DirectIndexedX,
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

		static const std::unordered_map<Byte, Cycles(CPU::*)()> SINGLE_BYTE_INSTRUCTIONS;
		static const std::unordered_map<Byte, std::pair<Cycles(CPU::*)(AddressingMode), Blaze::CPU::AddressingMode>> SINGLE_BYTE_INSTRUCTIONS_WITH_MODE;
		static const std::unordered_map<Byte, std::pair<Byte, Cycles(CPU::*)()>> SPECIAL_MULTI_BYTE_INSTRUCTIONS;
		static const std::unordered_map<Byte, std::tuple<Byte, Cycles(CPU::*)(AddressingMode), Blaze::CPU::AddressingMode>> SPECIAL_MULTI_BYTE_INSTRUCTIONS_WITH_MODE;

		struct flags {
			// Process Status Flags
			unsigned c: 1; // carry
			unsigned z: 1; // zero
			unsigned i: 1; // interrupt disable
			unsigned d: 1; // decimal
			unsigned x: 1; // index register width
			unsigned b: 1; // break
			unsigned m: 1; // accumulator & memory width
			unsigned v: 1; // overflow
			unsigned n: 1; // negative

			unsigned e: 1; // emulation mode
		} ; flags f;

		Word A; // accumulator
		Word DR; // direct
		Word PC; // program counter
		Word X, Y; // index registers
		Word SP; // stack pointer
		Byte DBR; // data bank
		Byte PBR; // program bank
		Byte P; // process status
		MemRam* _memory;

		Byte load8(Byte bank, Word addressLow) const;
		Word load16(Byte bank, Word addressLow) const;
		Address load24(Byte bank, Word addressLow) const;

		// this function is meant to be called by instruction execution functions to obtain the address
		// of the operand with the given addressing mode.
		Address decodeAddress(AddressingMode addressingMode) const;

		// executes the current instruction, stores the size of the instruction in `outInstructionSize`, and returns the number of cycles the instruction took
		Cycles executeInstruction(Byte& outInstructionSize);

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

	public:
		void reset(MemRam &memory);			// Reset CPU internal state
		void execute(ClockTicks cTicks);	// Execute the current instruction
		void clock();						// CPU driver
	};
} // namespace Blaze
