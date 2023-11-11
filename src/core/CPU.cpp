#include <blaze/CPU.hpp>
#include "blaze/Bus.hpp"
#include <cassert>
#include <blaze/util.hpp>

// TODO: fill in cycle info
const std::unordered_map<Blaze::Byte, Blaze::CPU::Instruction> Blaze::CPU::INSTRUCTIONS_WITH_NO_PATTERN {
	{ 0x40, Instruction(Opcode::RTI, 1, 0) },
	{ 0x60, Instruction(Opcode::RTS, 1, 0) },
	{ 0x08, Instruction(Opcode::PHP, 1, 0) },
	{ 0x28, Instruction(Opcode::PLP, 1, 0) },
	{ 0x48, Instruction(Opcode::PHA, 1, 0) },
	{ 0x68, Instruction(Opcode::PLA, 1, 0) },
	{ 0x88, Instruction(Opcode::DEY, 1, 0) },
	{ 0xa8, Instruction(Opcode::TAY, 1, 0) },
	{ 0xc8, Instruction(Opcode::INY, 1, 0) },
	{ 0xe8, Instruction(Opcode::INX, 1, 0) },
	{ 0x18, Instruction(Opcode::CLC, 1, 0) },
	{ 0x38, Instruction(Opcode::SEC, 1, 0) },
	{ 0x58, Instruction(Opcode::CLI, 1, 0) },
	{ 0x78, Instruction(Opcode::SEI, 1, 0) },
	{ 0x98, Instruction(Opcode::TYA, 1, 0) },
	{ 0xb8, Instruction(Opcode::CLV, 1, 0) },
	{ 0xd8, Instruction(Opcode::CLD, 1, 0) },
	{ 0xf8, Instruction(Opcode::SED, 1, 0) },
	{ 0x8a, Instruction(Opcode::TXA, 1, 0) },
	{ 0x9a, Instruction(Opcode::TXS, 1, 0) },
	{ 0xaa, Instruction(Opcode::TAX, 1, 0) },
	{ 0xba, Instruction(Opcode::TSX, 1, 0) },
	{ 0xca, Instruction(Opcode::DEX, 1, 0) },
	{ 0xea, Instruction(Opcode::NOP, 1, 0) },
	{ 0x5a, Instruction(Opcode::PHY, 1, 0) },
	{ 0x7a, Instruction(Opcode::PLY, 1, 0) },
	{ 0xda, Instruction(Opcode::PHX, 1, 0) },
	{ 0xfa, Instruction(Opcode::PLX, 1, 0) },
	{ 0x0b, Instruction(Opcode::PHD, 1, 0) },
	{ 0x2b, Instruction(Opcode::PLD, 1, 0) },
	{ 0x4b, Instruction(Opcode::PHK, 1, 0) },
	{ 0x6b, Instruction(Opcode::RTL, 1, 0) },
	{ 0x8b, Instruction(Opcode::PHB, 1, 0) },
	{ 0xab, Instruction(Opcode::PLB, 1, 0) },
	{ 0xcb, Instruction(Opcode::WAI, 1, 0) },
	{ 0xeb, Instruction(Opcode::XBA, 1, 0) },
	{ 0x1b, Instruction(Opcode::TCS, 1, 0) },
	{ 0x3b, Instruction(Opcode::TSC, 1, 0) },
	{ 0x5b, Instruction(Opcode::TCD, 1, 0) },
	{ 0x7b, Instruction(Opcode::TDC, 1, 0) },
	{ 0x9b, Instruction(Opcode::TXY, 1, 0) },
	{ 0xbb, Instruction(Opcode::TYX, 1, 0) },
	{ 0xdb, Instruction(Opcode::STP, 1, 0) },
	{ 0xfb, Instruction(Opcode::XCE, 1, 0) },
	{ 0x42, Instruction(Opcode::WDM, 2, 0) },

	{ 0x1a, Instruction(Opcode::INC, 1, 0, AddressingMode::Accumulator) },
	{ 0x3a, Instruction(Opcode::DEC, 1, 0, AddressingMode::Accumulator) },

	{ 0x00, Instruction(Opcode::BRK, 2, 0) }, // requires 2 bytes, but the 2nd byte is unused
	{ 0x02, Instruction(Opcode::COP, 2, 0) },
	{ 0x22, Instruction(Opcode::JSL, 4, 0, AddressingMode::AbsoluteLong) },
	{ 0x62, Instruction(Opcode::PER, 3, 0) },
	{ 0x82, Instruction(Opcode::BRL, 3, 0) },
	{ 0xc2, Instruction(Opcode::REP, 2, 0, AddressingMode::Immediate) },
	{ 0xe2, Instruction(Opcode::SEP, 2, 0, AddressingMode::Immediate) },
	{ 0x44, Instruction(Opcode::MVP, 3, 0, AddressingMode::BlockMove) },
	{ 0x54, Instruction(Opcode::MVN, 3, 0, AddressingMode::BlockMove) },
	{ 0xd4, Instruction(Opcode::PEI, 2, 0) },
	{ 0xf4, Instruction(Opcode::PEA, 3, 0) },
	{ 0xdc, Instruction(Opcode::JML, 3, 0, AddressingMode::AbsoluteIndirect) },

	{ 0x20, Instruction(Opcode::JSR, 3, 0, AddressingMode::Absolute) },
	{ 0x89, Instruction(Opcode::BIT, 2, 0, AddressingMode::Immediate) },
	{ 0x14, Instruction(Opcode::TRB, 2, 0, AddressingMode::Direct) },
	{ 0x1c, Instruction(Opcode::TRB, 3, 0, AddressingMode::Absolute) },
	{ 0x64, Instruction(Opcode::STZ, 2, 0, AddressingMode::Direct) },
	{ 0x9c, Instruction(Opcode::STZ, 3, 0, AddressingMode::Absolute) },
	{ 0x74, Instruction(Opcode::STZ, 2, 0, AddressingMode::DirectIndexedX) },
	{ 0x9e, Instruction(Opcode::STZ, 3, 0, AddressingMode::AbsoluteIndexedX) },
	{ 0x5c, Instruction(Opcode::JMP, 4, 0, AddressingMode::AbsoluteLong) },
	{ 0xfc, Instruction(Opcode::JSR, 3, 0, AddressingMode::AbsoluteIndexedIndirect) },
};

// NOLINTBEGIN(readability-magic-numbers, readability-identifier-length)
static constexpr Blaze::Byte opcodeGetGroupSelect(Blaze::Byte opcode) {
	return opcode & 0x03;
};

static constexpr Blaze::Byte opcodeGetAddressingMode(Blaze::Byte opcode) {
	return (opcode >> 2) & 0x07;
};

static constexpr Blaze::Byte opcodeGetSubopcode(Blaze::Byte opcode) {
	return (opcode >> 5) & 0x07;
};
// NOLINTEND(readability-magic-numbers, readability-identifier-length)

void Blaze::CPU::reset(MemRam &memory) {
	PC = load16(ExceptionVectorAddress::EmulatedRESET); // need to load w/contents of reset vector
	DBR = PBR = 0x00;
	A.reset();
	X.reset();
	Y.reset();
	SP = 0x0100;

	setFlag(flags::d, false);

	setFlag(flags::m, true);
	setFlag(flags::x, true);
	setFlag(flags::i, true);
	setFlag(flags::c, true);

	// the processor starts out in emulation mode
	e = 1;

	_memory = &memory;
	_memory->reset();
}

void Blaze::CPU::irq() {
	// If the interrupt is not masked
	if (!getFlag(flags::i))
	{
		if (!usingEmulationMode()) {
			// in native mode: push the PBR
			store8(SP, PBR);
			SP--;
		}

		// Push the value of pc onto the stack
		store16(SP - 1, PC);
		SP -= 2;

		// Set some interrupt flags first and then push the status register onto the stack
		setFlag(flags::b, false);
		setFlag(flags::i, true);
		store8(SP, P);
		SP--;

		// the PBR is forced to 0
		PBR = 0;

		// Read the interrupt program address from the interrupt table
		PC = load16(usingEmulationMode() ? ExceptionVectorAddress::EmulatedIRQ : ExceptionVectorAddress::NativeIRQ);

		// Handling IRQs takes 7 CPU cycles
		cyclesCountDown = 7;
	}
}

void Blaze::CPU::nmi() {
	if (!usingEmulationMode()) {
		// in native mode: push the PBR
		store8(SP, PBR);
		SP--;
	}

	store16(SP - 1, PC);
	SP -= 2;

	setFlag(flags::b, false);
	setFlag(flags::i, true);
	store8(SP, P);
	SP--;

	// the PBR is forced to 0
	PBR = 0;

	PC = load16(usingEmulationMode() ? ExceptionVectorAddress::EmulatedNMI : ExceptionVectorAddress::NativeNMI);

	cyclesCountDown = 8;
}

void Blaze::CPU::abort() {
	store8(SP, PBR);
	SP--;

	store16(SP - 1, PC);
	SP -= 2;

	store8(SP, P);
	SP--;

	setFlag(flags::i, true);
	setFlag(flags::d, false);

	PBR = 0x00;

	PC = load16(usingEmulationMode() ? ExceptionVectorAddress::EmulatedABORT : ExceptionVectorAddress::NativeABORT);

	cyclesCountDown = 8;
}

void Blaze::CPU::setZeroNegFlags(const Register& reg) {
	setFlag(flags::n, reg.mostSignificantBit());
	setFlag(flags::z, reg.load() == 0);
}

void Blaze::CPU::setOverflowFlag(Word leftOperand, Word rightOperand, Word result) {
	bool msb8Bit = memoryAndAccumulatorAre8Bit();
	bool leftSign = msb(leftOperand, msb8Bit);
	bool rightSign = msb(rightOperand, msb8Bit);
	bool resultSign = msb(result, msb8Bit);
	// if the signs of the inputs are equal to each other but not the sign of the result,
	// then signed overflow/underflow occurred.
	setFlag(flags::v, (leftSign == rightSign) && (leftSign != resultSign));
};

void Blaze::CPU::execute() {

	// Read first 8 bytes of next instruction
	//Byte* instruction = bus->read(PC);

	// update `executingPC` to point to the instruction we're about to execute
	executingPC = concat24(PBR, PC);

	// decode instruction and get info (e.g. # of cycles to run, instruction size)
	auto info = decodeInstruction(load8(executingPC));

	// the PC is always incremented to the next instruction before the current instruction starts executing
	PC += info.size;

	// execute instruction with the info
	info.cycles = executeInstruction(info);

	// Count down cycles
	while(info.cycles > 0)
	{
		--info.cycles;
	}
}

void Blaze::CPU::setFlag(Byte flag, bool s) {
	if (s) {
		P |= flag; // set flag
	} else {
		P &= ~flag; // clear flag
	}
}

bool Blaze::CPU::getFlag(Byte f) const {
	return (P & f) != 0;
};

Blaze::Byte Blaze::CPU::load8(Address address) const {
	return bus->read8(address);
};

Blaze::Word Blaze::CPU::load16(Address address) const {
	return bus->read16(address);
};

Blaze::Address Blaze::CPU::load24(Address address) const {
	return bus->read24(address);
};

Blaze::Byte Blaze::CPU::load8(Byte bank, Word addressLow) const {
	return load8(concat24(bank, addressLow));
};

Blaze::Word Blaze::CPU::load16(Byte bank, Word addressLow) const {
	return load16(concat24(bank, addressLow));
};

Blaze::Address Blaze::CPU::load24(Byte bank, Word addressLow) const {
	return load24(concat24(bank, addressLow));
};

void Blaze::CPU::store8(Address address, Byte value) {
	// Write address and value to bus
	bus->write(address, value);
};

void Blaze::CPU::store16(Address address, Word value) {
	// Write 16-bit value to address through bus
	bus->write(address, value);
};

void Blaze::CPU::store24(Address address, Address value) {
	// Write 24-bit value to address through bus
	bus->write(address, value);
};

void Blaze::CPU::store8(Byte bank, Word addressLow, Byte value) {
	return store8(concat24(bank, addressLow), value);
};

void Blaze::CPU::store16(Byte bank, Word addressLow, Word value) {
	return store16(concat24(bank, addressLow), value);
};

void Blaze::CPU::store24(Byte bank, Word addressLow, Address value) {
	return store24(concat24(bank, addressLow), value);
};

Blaze::Address Blaze::CPU::decodeAddress(AddressingMode mode) const {
	Address addressStart = executingPC + 1;

	switch (mode) {
		case AddressingMode::Absolute:
			return concat24(DBR, load16(addressStart));
		case AddressingMode::AbsoluteIndexedIndirect:
			return load16(0, load16(addressStart) + X.load());
		case AddressingMode::AbsoluteIndexedX:
			return concat24(DBR, load16(addressStart) + X.load());
		case AddressingMode::AbsoluteIndexedY:
			return concat24(DBR, load16(addressStart) + Y.load());

		case AddressingMode::AbsoluteIndirect: {
			auto base = load16(addressStart);
			if (load8(executingPC) == /* JML */ 0xdc) {
				return load24(0, base);
			} else {
				return load16(0, base);
			}
		} break;

		case AddressingMode::AbsoluteLongIndexedX:
			return load24(addressStart) + X.load();
		case AddressingMode::AbsoluteLong:
			return load24(addressStart);
		case AddressingMode::DirectIndexedIndirect:
			return concat24(DBR, load16(0, DR + X.load() + load8(addressStart)));
		case AddressingMode::DirectIndexedX:
			return concat24(0, DR + X.load() + load8(addressStart));
		case AddressingMode::DirectIndexedY:
			return concat24(0, DR + Y.load() + load8(addressStart));
		case AddressingMode::DirectIndirectIndexed:
			return concat24(DBR, load16(0, DR + load8(addressStart))) + Y.load();
		case AddressingMode::DirectIndirectLongIndexed:
			return load24(0, DR + load8(addressStart)) + Y.load();
		case AddressingMode::DirectIndirectLong:
			return load24(0, DR + load8(addressStart));
		case AddressingMode::DirectIndirect:
			return concat24(DBR, load16(0, DR + load8(addressStart)));
		case AddressingMode::Direct:
			return concat24(0, DR + load8(addressStart));
		case AddressingMode::ProgramCounterRelativeLong:
			// `PC + 3` since the PC used for the calculation is the address of the *next* instruction
			return static_cast<uint16_t>(static_cast<int16_t>(PC + 3) + static_cast<int16_t>(load16(addressStart)));
		case AddressingMode::ProgramCounterRelative:
			// ditto
			return static_cast<uint16_t>(static_cast<int16_t>(PC + 2) + static_cast<int8_t>(load8(addressStart)));
		case AddressingMode::StackRelative:
			return concat24(0, SP + load8(addressStart));
		case AddressingMode::StackRelativeIndirectIndexed:
			return concat24(DBR, load16(0, SP + load8(addressStart))) + Y.load();

		case AddressingMode::Accumulator:
		case AddressingMode::BlockMove:
		case AddressingMode::Immediate:
		case AddressingMode::Implied:
		case AddressingMode::Stack:
		default:
			return 0;
	}
};

Blaze::Word Blaze::CPU::loadOperand(AddressingMode addressingMode) const {
	Address operand = decodeAddress(addressingMode);
	if (addressingMode != AddressingMode::Immediate) {
		operand = load16(operand);
	}

	// make sure the operand is actually 16 bits wide and not 24 bits
	assert((operand & 0xffff) == operand);

	return operand;
};

// special thanks to https://llx.com/Neil/a2/opcodes.html for some wisdom on how to intelligently decode the instructions
// (without having a giant switch statement)
Blaze::CPU::Instruction Blaze::CPU::decodeInstruction(Byte inst0) const {
	// before doing any smart decoding, we first do some simple opcode comparisons.
	// there are some instructions that only require a single byte (their opcode).
	// then there are those instructions that require multiple bytes, but have no
	// clear pattern that can be used to decode them "intelligently".

	auto entry = INSTRUCTIONS_WITH_NO_PATTERN.find(inst0);
	if (entry != INSTRUCTIONS_WITH_NO_PATTERN.end()) {
		return entry->second;
	}

	// this is a super special case
	if (inst0 == 0x80) {
		return Instruction(Opcode::BRA, 2, 0, ConditionCode::NONE, false);
	}

	// for the rest of the opcodes, we decode them according to patterns

	auto groupSelect = opcodeGetGroupSelect(inst0);
	auto namespacedAddrMode = opcodeGetAddressingMode(inst0);
	auto namespacedOpcode = opcodeGetSubopcode(inst0);

	// NOTE: the namespaced address mode needs to be verified for the particular group,
	//       since not all possible values (0 through and including 7) are valid in
	//       all groups, nor with all opcodes within each group.
	//
	//       the namespaced opcode is ALWAYS valid since all possible values (0 through
	//       and including 7) are valid in all groups.

	// we use the 2 lowest bits as a selector to determine if the instruction is
	// in Group 1, Group 2, or Group 3.
	switch (groupSelect) {
		case 1: {
			// Group 1
			AddressingMode mode = GROUP1_6502_ADDRESS_MODE_MAP[namespacedAddrMode];
			auto instructionSize = instructionSizeWithAddressingMode(mode);
			auto opcode = static_cast<Group1Opcode>(namespacedOpcode);

			// in this group, all addressing mode values are valid, so no need to check that.

			if (mode == AddressingMode::Immediate && opcode == Group1Opcode::STA) {
				return Instruction();
			}

			if (instructionSize == 0) {
				// there are only 2 cases where this happens: Immediate and Stack
				// Stack is impossible because it's not supported in this group,
				// so it must be Immediate. in this case, the instruction size depends
				// on the CPU flags.
				//
				// when the `m` flag is unset, it takes up 3 bytes instead of 2.
				instructionSize = !getFlag(flags::m) ? 3 : 2;
			}

			switch (opcode) {
				case Group1Opcode::ORA: return Instruction(Opcode::ORA, instructionSize, 0, mode);
				case Group1Opcode::AND: return Instruction(Opcode::AND, instructionSize, 0, mode);
				case Group1Opcode::EOR: return Instruction(Opcode::EOR, instructionSize, 0, mode);
				case Group1Opcode::ADC: return Instruction(Opcode::ADC, instructionSize, 0, mode);
				case Group1Opcode::STA: return Instruction(Opcode::STA, instructionSize, 0, mode);
				case Group1Opcode::LDA: return Instruction(Opcode::LDA, instructionSize, 0, mode);
				case Group1Opcode::CMP: return Instruction(Opcode::CMP, instructionSize, 0, mode);
				case Group1Opcode::SBC: return Instruction(Opcode::SBC, instructionSize, 0, mode);
			}
		} break;

		case 2: {
			// Group 2

			// if the address mode matches the special 65C02 address mode, we process it as a Group 1 instruction instead.
			if (namespacedAddrMode == GROUP2_65C02_ADDRESS_MODE) {
				switch (static_cast<Group1Opcode>(namespacedOpcode)) {
					case Group1Opcode::ORA: return Instruction(Opcode::ORA, 2, 0, AddressingMode::DirectIndirect);
					case Group1Opcode::AND: return Instruction(Opcode::AND, 2, 0, AddressingMode::DirectIndirect);
					case Group1Opcode::EOR: return Instruction(Opcode::EOR, 2, 0, AddressingMode::DirectIndirect);
					case Group1Opcode::ADC: return Instruction(Opcode::ADC, 2, 0, AddressingMode::DirectIndirect);
					case Group1Opcode::STA: return Instruction(Opcode::STA, 2, 0, AddressingMode::DirectIndirect);
					case Group1Opcode::LDA: return Instruction(Opcode::LDA, 2, 0, AddressingMode::DirectIndirect);
					case Group1Opcode::CMP: return Instruction(Opcode::CMP, 2, 0, AddressingMode::DirectIndirect);
					case Group1Opcode::SBC: return Instruction(Opcode::SBC, 2, 0, AddressingMode::DirectIndirect);
				}
			}

			AddressingMode mode = GROUP2_ADDRESS_MODE_MAP[namespacedAddrMode];
			auto instructionSize = instructionSizeWithAddressingMode(mode);
			auto opcode = static_cast<Group2Opcode>(namespacedOpcode);

			if (mode == AddressingMode::INVALID) {
				return Instruction();
			}

			// in this group, not all instructions support all valid modes

			if (
				// only LDX supports immediate addressing
				(mode == AddressingMode::Immediate && opcode != Group2Opcode::LDX) ||
				// STX doesn't support Absolute Indexed X addressing
				(mode == AddressingMode::AbsoluteIndexedX && opcode == Group2Opcode::STX)
			) {
				return Instruction();
			}

			if (mode == AddressingMode::Accumulator) {
				switch (opcode) {
					// STX, LDX, DEC, and INC don't support Accumulator addressing
					case Group2Opcode::STX:
					case Group2Opcode::LDX:
					case Group2Opcode::DEC:
					case Group2Opcode::INC:
						return Instruction();

					default:
						break;
				}
			}

			if (instructionSize == 0) {
				// same as for Group 1 instructions
				instructionSize = !getFlag(flags::m) ? 3 : 2;
			}

			switch (opcode) {
				case Group2Opcode::ASL: return Instruction(Opcode::ASL, instructionSize, 0, mode);
				case Group2Opcode::ROL: return Instruction(Opcode::ROL, instructionSize, 0, mode);
				case Group2Opcode::LSR: return Instruction(Opcode::LSR, instructionSize, 0, mode);
				case Group2Opcode::ROR: return Instruction(Opcode::ROR, instructionSize, 0, mode);
				case Group2Opcode::STX: return Instruction(Opcode::STX, instructionSize, 0, mode);
				case Group2Opcode::LDX: return Instruction(Opcode::LDX, instructionSize, 0, mode);
				case Group2Opcode::DEC: return Instruction(Opcode::DEC, instructionSize, 0, mode);
				case Group2Opcode::INC: return Instruction(Opcode::INC, instructionSize, 0, mode);
			}
		} break;

		case 0: {
			// Group 3

			// if the address mode matches the special condition address mode, we process it as a branch instruction with a condition.
			if (namespacedAddrMode == GROUP3_CONDITION_ADDRESS_MODE) {
				return Instruction(Opcode::BRA, 2, 0, static_cast<ConditionCode>(namespacedOpcode >> 1), (namespacedOpcode & 0x01) != 0);
			}

			AddressingMode mode = GROUP3_ADDRESS_MODE_MAP[namespacedAddrMode];
			auto instructionSize = instructionSizeWithAddressingMode(mode);
			auto opcode = static_cast<Group3Opcode>(namespacedOpcode);

			if (mode == AddressingMode::INVALID) {
				return Instruction();
			}

			// in this group, not all instructions support all valid modes

			if (
				// JMP and JMPIndirect don't support Direct addressing
				(mode == AddressingMode::Direct && (opcode == Group3Opcode::JMP || opcode == Group3Opcode::JMPIndirect)) ||
				// only STY, LDY, and BIT support Direct Indexed X addressing
				(mode == AddressingMode::DirectIndexedX && opcode != Group3Opcode::STY && opcode != Group3Opcode::LDY && opcode != Group3Opcode::BIT) ||
				// only LDY, BIT, and JMPIndirect support Absolute Indexed X addressing
				(mode == AddressingMode::AbsoluteIndexedX && opcode != Group3Opcode::LDY && opcode != Group3Opcode::BIT && opcode != Group3Opcode::JMPIndirect) ||
				// only LDY, CPY, and CPX support Immediate addressing
				(mode == AddressingMode::Immediate && opcode != Group3Opcode::LDY && opcode != Group3Opcode::CPY && opcode != Group3Opcode::CPX)
			) {
				return Instruction();
			}

			if (instructionSize == 0) {
				// same as for Group 1 instructions
				instructionSize = !getFlag(flags::m) ? 3 : 2;
			}

			switch (opcode) {
				case Group3Opcode::TSB:         return Instruction(Opcode::TSB, instructionSize, 0, mode);
				case Group3Opcode::BIT:         return Instruction(Opcode::BIT, instructionSize, 0, mode);
				case Group3Opcode::JMP:         return Instruction(Opcode::JMP, instructionSize, 0, mode);
				case Group3Opcode::STY:         return Instruction(Opcode::STY, instructionSize, 0, mode);
				case Group3Opcode::LDY:         return Instruction(Opcode::LDY, instructionSize, 0, mode);
				case Group3Opcode::CPY:         return Instruction(Opcode::CPY, instructionSize, 0, mode);
				case Group3Opcode::CPX:         return Instruction(Opcode::CPX, instructionSize, 0, mode);
				case Group3Opcode::JMPIndirect:
					// this is a special case, because we have to take the given
					// addressing mode and use the indirect version of it
					switch (mode) {
						case AddressingMode::Absolute:         return Instruction(Opcode::JMP, instructionSize, 0, AddressingMode::AbsoluteIndirect);
						case AddressingMode::AbsoluteIndexedX: return Instruction(Opcode::JMP, instructionSize, 0, AddressingMode::AbsoluteIndexedIndirect);

						default:
							return Instruction();
					}
					break;
			}
		} break;

		case 3: {
			// Group 1 but with new 65C816 addressing modes
			AddressingMode mode = GROUP1_65C816_ADDRESS_MODE_MAP[namespacedAddrMode];
			auto instructionSize = instructionSizeWithAddressingMode(mode);
			auto opcode = static_cast<Group1Opcode>(namespacedOpcode);

			// in this group, all addressing mode values are valid, so no need to check that.

			if (instructionSize == 0) {
				// same as for Group 1 instructions
				instructionSize = !getFlag(flags::m) ? 3 : 2;
			}

			switch (opcode) {
				case Group1Opcode::ORA: return Instruction(Opcode::ORA, instructionSize, 0, mode);
				case Group1Opcode::AND: return Instruction(Opcode::AND, instructionSize, 0, mode);
				case Group1Opcode::EOR: return Instruction(Opcode::EOR, instructionSize, 0, mode);
				case Group1Opcode::ADC: return Instruction(Opcode::ADC, instructionSize, 0, mode);
				case Group1Opcode::STA: return Instruction(Opcode::STA, instructionSize, 0, mode);
				case Group1Opcode::LDA: return Instruction(Opcode::LDA, instructionSize, 0, mode);
				case Group1Opcode::CMP: return Instruction(Opcode::CMP, instructionSize, 0, mode);
				case Group1Opcode::SBC: return Instruction(Opcode::SBC, instructionSize, 0, mode);
			}
		} break;

		default:
			return Instruction();
	}
};

Blaze::Cycles Blaze::CPU::executeInstruction(const Instruction& info) {
	switch (info.opcode) {
		case Opcode::BRK: return executeBRK();
		case Opcode::BRL: return executeBRL();
		case Opcode::CLC: return executeCLC();
		case Opcode::CLD: return executeCLD();
		case Opcode::CLI: return executeCLI();
		case Opcode::CLV: return executeCLV();
		case Opcode::COP: return executeCOP();
		case Opcode::DEX: return executeDEX();
		case Opcode::DEY: return executeDEY();
		case Opcode::INX: return executeINX();
		case Opcode::INY: return executeINY();
		case Opcode::JML: return executeJML();
		case Opcode::JSL: return executeJSL();
		case Opcode::MVN: return executeMVN();
		case Opcode::MVP: return executeMVP();
		case Opcode::NOP: return executeNOP();
		case Opcode::PEA: return executePEA();
		case Opcode::PEI: return executePEI();
		case Opcode::PER: return executePER();
		case Opcode::PHA: return executePHA();
		case Opcode::PHB: return executePHB();
		case Opcode::PHD: return executePHD();
		case Opcode::PHK: return executePHK();
		case Opcode::PHP: return executePHP();
		case Opcode::PHX: return executePHX();
		case Opcode::PHY: return executePHY();
		case Opcode::PLA: return executePLA();
		case Opcode::PLB: return executePLB();
		case Opcode::PLD: return executePLD();
		case Opcode::PLP: return executePLP();
		case Opcode::PLX: return executePLX();
		case Opcode::PLY: return executePLY();
		case Opcode::REP: return executeREP();
		case Opcode::RTI: return executeRTI();
		case Opcode::RTL: return executeRTL();
		case Opcode::RTS: return executeRTS();
		case Opcode::SEC: return executeSEC();
		case Opcode::SED: return executeSED();
		case Opcode::SEI: return executeSEI();
		case Opcode::SEP: return executeSEP();
		case Opcode::STP: return executeSTP();
		case Opcode::TAX: return executeTAX();
		case Opcode::TAY: return executeTAY();
		case Opcode::TCD: return executeTCD();
		case Opcode::TCS: return executeTCS();
		case Opcode::TDC: return executeTDC();
		case Opcode::TSC: return executeTSC();
		case Opcode::TSX: return executeTSX();
		case Opcode::TXA: return executeTXA();
		case Opcode::TXS: return executeTXS();
		case Opcode::TXY: return executeTXY();
		case Opcode::TYA: return executeTYA();
		case Opcode::TYX: return executeTYX();
		case Opcode::WAI: return executeWAI();
		case Opcode::WDM: return executeWDM();
		case Opcode::XBA: return executeXBA();
		case Opcode::XCE: return executeXCE();

		case Opcode::ADC: return executeADC(info.addressingMode);
		case Opcode::AND: return executeAND(info.addressingMode);
		case Opcode::ASL: return executeASL(info.addressingMode);
		case Opcode::BIT: return executeBIT(info.addressingMode);
		case Opcode::CMP: return executeCMP(info.addressingMode);
		case Opcode::CPX: return executeCPX(info.addressingMode);
		case Opcode::CPY: return executeCPY(info.addressingMode);
		case Opcode::DEC: return executeDEC(info.addressingMode);
		case Opcode::EOR: return executeEOR(info.addressingMode);
		case Opcode::INC: return executeINC(info.addressingMode);
		case Opcode::JMP: return executeJMP(info.addressingMode);
		case Opcode::JSR: return executeJSR(info.addressingMode);
		case Opcode::LDA: return executeLDA(info.addressingMode);
		case Opcode::LDX: return executeLDX(info.addressingMode);
		case Opcode::LDY: return executeLDY(info.addressingMode);
		case Opcode::LSR: return executeLSR(info.addressingMode);
		case Opcode::ORA: return executeORA(info.addressingMode);
		case Opcode::ROL: return executeROL(info.addressingMode);
		case Opcode::ROR: return executeROR(info.addressingMode);
		case Opcode::SBC: return executeSBC(info.addressingMode);
		case Opcode::STA: return executeSTA(info.addressingMode);
		case Opcode::STX: return executeSTX(info.addressingMode);
		case Opcode::STY: return executeSTY(info.addressingMode);
		case Opcode::STZ: return executeSTZ(info.addressingMode);
		case Opcode::TRB: return executeTRB(info.addressingMode);
		case Opcode::TSB: return executeTSB(info.addressingMode);

		case Opcode::BRA: return executeBRA(info.condition, info.passConditionIfBitSet);

		default:
			return invalidInstruction();
	}
};

Blaze::Cycles Blaze::CPU::invalidInstruction() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeBRK() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeBRL() {
	PC = decodeAddress(AddressingMode::ProgramCounterRelativeLong);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCLC() {
	setFlag(flags::c, false);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCLD() {
	setFlag(flags::d, false);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCLI() {
	setFlag(flags::i, false);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCLV() {
	setFlag(flags::v, false);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCOP() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeDEX() {
	X--;
	setZeroNegFlags(X);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeDEY() {
	Y--;
	setZeroNegFlags(Y);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeINX() {
	X++;
	setZeroNegFlags(X);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeINY() {
	Y++;
	setZeroNegFlags(Y);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeJML() {
	Address addr = decodeAddress(AddressingMode::AbsoluteIndirect);
	split24(addr, PBR, PC);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeJSL() {
	Address newPC = decodeAddress(AddressingMode::AbsoluteLong);
	// subtract 1 because it's required
	Address pcToStore = concat24(PBR, PC - 1);

	SP -= 2;
	store24(SP, pcToStore);
	--SP;

	split24(newPC, PBR, PC);

	return 0;
};

Blaze::Cycles Blaze::CPU::executeMVN() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeMVP() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeNOP() {
	return 0;
};

Blaze::Cycles Blaze::CPU::executePEA() {
	Word address = load16(PC + 1);
	SP -= 2;
	store16(0, SP + 1, address);
	return 0;
};

Blaze::Cycles Blaze::CPU::executePEI() {
	Word effectiveAddress = decodeAddress(AddressingMode::DirectIndirect);
	SP -= 2;
	store16(0, SP + 1, effectiveAddress);
	//SP -= 2;
	return 0;
};

Blaze::Cycles Blaze::CPU::executePER() {
	Word relativeAddr = decodeAddress(AddressingMode::ProgramCounterRelativeLong);
	SP -= 2;
	store16(0, SP + 1, relativeAddr);
	//SP -= 2;
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHA() {
	if (memoryAndAccumulatorAre8Bit()) {
		store8(SP, A.load());
	}
	else {
		SP--;
		store16(SP, A.forceLoadFull());
	}
	SP--;
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHB() {
	store8(SP, DBR);
	SP--;
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHD() {
	if (usingEmulationMode()) {
		store8(SP, DR);
	} else {
		SP--;
		store16(SP, DR);
	}
	SP--;
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHK() {
	store8(SP, PBR);
	SP--;
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHP() {
	store8(SP, P);
	SP--;
	setFlag(flags::b, false);
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHX() {
	if (indexRegistersAre8Bit()) {
		store8(SP, X.load());
	}
	else {
		SP--;
		store16(SP, X.forceLoadFull());
	}
	SP--;
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHY() {
	if (indexRegistersAre8Bit()) {
		store8(SP, Y.load());
	}
	else {
		SP--;
		store16(SP, Y.forceLoadFull());
	}
	SP--;
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLA() {
	SP++;
	if (memoryAndAccumulatorAre8Bit()) {
		A = load8(SP);
	}
	else {
		A = load16(SP);
		SP++;
	}
	setZeroNegFlags(A);
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLB() {
	SP++;
	DBR = load8(SP);
	setFlag(flags::n, msb8(DBR));
	setFlag(flags::z, (DBR == 0));
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLD() {
	SP++;
	if (usingEmulationMode()) {
		DR = load8(SP);
		setFlag(flags::n, msb8(DR));
	} else {
		DR = load16(SP);
		SP++;
		setFlag(flags::n, msb16(DR));
	}
	setFlag(flags::z, (DR == 0));
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLP() {
	SP++;
	P = load8(SP);
	if (usingEmulationMode()) {
		setFlag(flags::x, true);
		setFlag(flags::m, true);
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLX() {
	SP++;
	if (indexRegistersAre8Bit()) {
		X = load8(SP);
	}
	else {
		X = load16(SP);
		SP++;
	}
	setZeroNegFlags(X);
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLY() {
	SP++;
	if (indexRegistersAre8Bit()) {
		Y = load8(SP);
	}
	else {
		Y = load16(SP);
		SP++;
	}
	setZeroNegFlags(Y);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeREP() {
	Word val = loadOperand(AddressingMode::Immediate);
	P &= ~val;
	if (usingEmulationMode()) {
		setFlag(flags::x, true);
		setFlag(flags::m, true);
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeRTI() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeRTL() {
	++SP;
	Address newPC = load24(SP);
	SP += 2;

	split24(newPC, PBR, PC);
	++PC; // add 1 to account for the `- 1` when storing the PC (it's required)

	return 0;
};

Blaze::Cycles Blaze::CPU::executeRTS() {
	++SP;
	Address newPC = load16(SP);
	++SP;

	// add 1 to account for the `- 1` when storing the PC (it's required)
	PC = newPC + 1;

	return 0;
};

Blaze::Cycles Blaze::CPU::executeSEC() {
	setFlag(flags::c, true);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSED() {
	setFlag(flags::d, true);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSEI() {
	setFlag(flags::i, true);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSEP() {
	Word val = loadOperand(AddressingMode::Immediate);
	P |= val;
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSTP() {
	// Do nothing until there is an interrup
	while(true)
	{
		// Check for interrupt
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTAX() {
	X = A.forceLoadFull();
	setZeroNegFlags(X);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTAY() {
	Y = A.forceLoadFull();
	setZeroNegFlags(Y);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTCD() {
	DR = A.forceLoadFull();
	if (usingEmulationMode()) {
		setFlag(flags::n, msb8(DR));
	} else {
		setFlag(flags::n, msb16(DR));
	}
	setFlag(flags::z, (DR == 0));
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTCS() {
	SP = A.forceLoadFull();
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTDC() {
	A.forceStoreFull(DR);
	setZeroNegFlags(A);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTSC() {
	A.forceStoreFull(SP);
	setZeroNegFlags(A);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTSX() {
	X = SP;
	setZeroNegFlags(X);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTXA() {
	A = X.load();
	setZeroNegFlags(A);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTXS() {
	if (usingEmulationMode()) {
		SP = 0x0100 | lo8(X.load());
	} else {
		SP = X.load();
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTXY() {
	Y = X.load();
	setZeroNegFlags(Y);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTYA() {
	A = Y.load();
	setZeroNegFlags(A);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTYX() {
	X = Y.load();
	setZeroNegFlags(X);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeWAI() {
	// wait until there is an interrupt
	while(true)
	{
		// Interrupt mask is set: continue with next instruction and then interrupt
		//if()//There is an interrupt
		{
			if(flags::i)
			{
				// launch next instruction before going to interrupt
			}
			else
			{

			}
		}
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeWDM() {
	Byte operand = loadOperand(AddressingMode::Immediate);

	switch (operand) {
		case CustomWDMOpcodes::PutChararacter: {
			if (putCharacterHook) {
				putCharacterHook(static_cast<char>(lo8(A.load())));
			}
			return 0;
		} break;

		default:
			return invalidInstruction();
	}
};

Blaze::Cycles Blaze::CPU::executeXBA() {
	// get high and low bytes
	Word highMask = hi8(A.forceLoadFull(), false);
	Word lowMask = lo8(A.forceLoadFull());

	// Swap
	highMask = (highMask >> 8);
	lowMask = (lowMask << 8);

	// Store in A
	A.forceStoreFull(lowMask | highMask);

	return 0;
};

Blaze::Cycles Blaze::CPU::executeXCE() {
	// Swap values of e and c flags
	Byte tempC = getFlag(flags::c) ? 1 : 0;
	setFlag(flags::c, usingEmulationMode());
	e = tempC;

	// If switched to emulation mode
	if(usingEmulationMode())
	{
		// Force m and x to 1
		setFlag(flags::m, true);
		setFlag(flags::x, true);

		// XH and YH are forced to $00
		X = lo8(X.load());
		Y = lo8(Y.load());

		// SH is forced to $01
		SP = lo8(SP) | 0x0100;
	}

	return 0;
};

Blaze::Cycles Blaze::CPU::executeADC(AddressingMode mode) {
	// use `Address` instead of `Word` so that we have extra bits to properly compute the carry
	Address left = A.load();
	Address right = loadOperand(mode);
	Address result = left + right + getCarry();
	Address wordMask = (memoryAndAccumulatorAre8Bit() ? 0xff : 0xffff);
	Word wordResult = result & wordMask;

	A = wordResult;

	setZeroNegFlags(A);
	setOverflowFlag(left, right, wordResult);
	setFlag(flags::c, (result & ~wordMask) != 0);

	return 0;
};

Blaze::Cycles Blaze::CPU::executeAND(AddressingMode mode) {
	Word val = loadOperand(mode);
	A &= val;
	setZeroNegFlags(A);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeASL(AddressingMode mode) {
	// Get byte to shift
	Address addr = decodeAddress(mode);

	Word val;

	if (mode == AddressingMode::Accumulator) {
		val = A.load();
	} else if (memoryAndAccumulatorAre8Bit()) {
		val = load8(addr);
	} else {
		val = load16(addr);
	}

	// Set carry flag if current left bit is 1
	setFlag(flags::c, msb(val, memoryAndAccumulatorAre8Bit()));

	// Shift
	val <<= 1;

	if (mode == AddressingMode::Accumulator) {
		A.store(val);
	} else if (memoryAndAccumulatorAre8Bit()) {
		store8(addr, val);
	} else {
		store16(addr, val);
	}

	setFlag(flags::z, val == 0);
	setFlag(flags::n, msb(val, memoryAndAccumulatorAre8Bit()));

	return 0;
};

Blaze::Cycles Blaze::CPU::executeBIT(AddressingMode mode) {
	Word val = loadOperand(mode);
	setFlag(flags::z, (A & val) == 0);
	setFlag(flags::n, msb(val, memoryAndAccumulatorAre8Bit()));
	if (memoryAndAccumulatorAre8Bit()) {
		setFlag(flags::v, ((val & (1u << 6)) != 0));
	}
	else {
		setFlag(flags::v, ((val & (1u << 14)) != 0));
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCMP(AddressingMode mode) {
	Word val = loadOperand(mode);
	Word temp = A.load() - val;
	setFlag(flags::z, (A == val));
	setFlag(flags::c, (A >= val));
	setFlag(flags::n, msb(temp, memoryAndAccumulatorAre8Bit()));
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCPX(AddressingMode mode) {
	Word val = loadOperand(mode);
	Word temp = X - val;
	setFlag(flags::z, (X == val));
	setFlag(flags::c, (X >= val));
	setFlag(flags::n, msb(temp, indexRegistersAre8Bit()));
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCPY(AddressingMode mode) {
	Word val = loadOperand(mode);
	Word temp = Y - val;
	setFlag(flags::z, (Y == val));
	setFlag(flags::c, (Y >= val));
	setFlag(flags::n, msb(temp, indexRegistersAre8Bit()));
	return 0;
};

Blaze::Cycles Blaze::CPU::executeDEC(AddressingMode mode) {
	Address addr = decodeAddress(mode);
	Word val;
	if (memoryAndAccumulatorAre8Bit()) {
		val = load8(addr);
		val--;
		store8(addr, lo8(val));
	}
	else {
		val = load16(addr);
		val--;
		store16(addr, val);
	}
	setFlag(flags::n, msb(val, memoryAndAccumulatorAre8Bit()));
	setFlag(flags::z, (val == 0));
	return 0;
};

Blaze::Cycles Blaze::CPU::executeEOR(AddressingMode mode) {
	Word val = loadOperand(mode);
	A ^= val;
	setZeroNegFlags(A);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeINC(AddressingMode mode) {
	Address addr = decodeAddress(mode);
	Word val;
	if (memoryAndAccumulatorAre8Bit()) {
		val = load8(addr);
		val++;
		store8(addr, lo8(val));
	}
	else {
		val = load16(addr);
		val++;
		store16(addr, val);
	}
	setFlag(flags::n, msb(val, memoryAndAccumulatorAre8Bit()));
	setFlag(flags::z, (val == 0));
	return 0;
};

Blaze::Cycles Blaze::CPU::executeJMP(AddressingMode mode) {
	Address addr = decodeAddress(mode);
	PC = addr;
	return 0;
};

Blaze::Cycles Blaze::CPU::executeJSR(AddressingMode mode) {
	Address newPC = decodeAddress(mode);
	// subtract 1 because it's required
	Word pcToStore = PC - 1;

	--SP;
	store16(SP, pcToStore);
	--SP;

	PC = newPC;

	return 0;
};

Blaze::Cycles Blaze::CPU::executeLDA(AddressingMode mode) {
	Word val = loadOperand(mode);
	A = val;
	setZeroNegFlags(A);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeLDX(AddressingMode mode) {
	Word val = loadOperand(mode);
	X = val;
	setZeroNegFlags(X);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeLDY(AddressingMode mode) {
	Word val = loadOperand(mode);
	Y = val;
	setZeroNegFlags(Y);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeLSR(AddressingMode mode) {
	Address addr = decodeAddress(mode);
	Word data = memoryAndAccumulatorAre8Bit() ? load8(addr) : load16(addr);

	setFlag(flags::c, (data & 0x01) != 0);

	data >>= 1;

	if (memoryAndAccumulatorAre8Bit()) {
		store8(addr, data);
	} else {
		store16(addr, data);
	}

	return 0;
};

Blaze::Cycles Blaze::CPU::executeORA(AddressingMode mode) {
	Word val = loadOperand(mode);
	A |= val;
	setZeroNegFlags(A);
	return 0;
};

Blaze::Cycles Blaze::CPU::executeROL(AddressingMode mode) {
	Address addr = decodeAddress(mode);
	Word data = memoryAndAccumulatorAre8Bit() ? load8(addr) : load16(addr);
	Byte carry = getCarry();

	//set c to most significant bit of data
	setFlag(flags::c, msb(data, memoryAndAccumulatorAre8Bit()));

	data = (data << 1) | carry; // shift carry to least significant bit of 'data'

	if (memoryAndAccumulatorAre8Bit()) {
		store8(addr, data);
	} else {
		store16(addr, data);
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeROR(AddressingMode mode) {
	Address addr = decodeAddress(mode);
	Word data = memoryAndAccumulatorAre8Bit() ? load8(addr) : load16(addr);
	Byte carry = getCarry();

	setFlag(flags::c, (data & 0x01) != 0);

	data = (data >> 1) | carry;

	if (memoryAndAccumulatorAre8Bit()) {
		store8(addr, data);
	} else {
		store16(addr, data);
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSBC(AddressingMode mode) {
	// Fetch initial accumulator
	Address left = A.load();

	// Get and (bitwise) negate the operand
	Address operand = ~(loadOperand(mode));

	// Compute
	Address result = left + operand + getCarry();

	// Handle different widths
	Address wordMask = (memoryAndAccumulatorAre8Bit() ? 0xff : 0xffff);
	Word wordResult = result & wordMask;

	// Update accumulator
	A = wordResult;

	// Set flags
	setZeroNegFlags(A);
	setOverflowFlag(left, operand, wordResult);
	setFlag(flags::c, (result & ~wordMask) != 0);

	return 0;
};

Blaze::Cycles Blaze::CPU::executeSTA(AddressingMode mode) {
	Address address = decodeAddress(mode);
	if (memoryAndAccumulatorAre8Bit()) {
		store8(address, A.load());
	} else {
		store16(address, A.load());
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSTX(AddressingMode mode) {
	Address addr = decodeAddress(mode);

	// Store the X register's value at the determined address.
	if (indexRegistersAre8Bit()) {
		store8(addr, static_cast<Byte>(X.load()));  // Storing only lower 8 bits of X register
	} else {
		store16(addr, X.forceLoadFull()); // Storing full 16 bits of X register
	}

	return 0;
};

Blaze::Cycles Blaze::CPU::executeSTY(AddressingMode mode) {
	Address addr = decodeAddress(mode);

	// Store the Y register's value at the determined address.
	if (indexRegistersAre8Bit()) {
		store8(addr, static_cast<Byte>(Y.load()));  // Storing only lower 8 bits of Y register
	} else {
		store16(addr, Y.forceLoadFull()); // Storing full 16 bits of Y register
	}

	return 0;
};

Blaze::Cycles Blaze::CPU::executeSTZ(AddressingMode mode) {
	Address addr = loadOperand(mode);
	if (memoryAndAccumulatorAre8Bit()) {
		store8(addr, 0);
	}
	else {
		store16(addr, 0);
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTRB(AddressingMode mode) {
	Address addr = decodeAddress(mode);
	Word val;
	if (memoryAndAccumulatorAre8Bit()) {
		val = load8(addr);
		setFlag(flags::z, (val & A.load()) == 0);
		val &= ~A.load();
		store8(addr, lo8(val));
	}
	else {
		val = load16(addr);
		setFlag(flags::z, (val & A.load()) == 0);
		val &= ~A.load();
		store16(addr, val);
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTSB(AddressingMode mode) {
	Address addr = decodeAddress(mode);
	Word val;
	if (memoryAndAccumulatorAre8Bit()) {
		val = load8(addr);
		setFlag(flags::z, (val & A.load()) == 0);
		val |= A.load();
		store8(addr, lo8(val));
	}
	else {
		val = load16(addr);
		setFlag(flags::z, (val & A.load()) == 0);
		val |= A.load();
		store16(addr, val);
	}
	return 0;
};

Blaze::Cycles Blaze::CPU::executeBRA(ConditionCode condition, bool passConditionIfBitSet) {
	// Get the new PC if condition and bit are met
	Word newPC = decodeAddress(AddressingMode::ProgramCounterRelative);
	bool bitIsSet = false;


	// No condition passed or condition == NONE -> BRA
	if (condition == ConditionCode::NONE) {
		// update PC
		PC = newPC;
	} else {
		// Check the correct bit based on 
		switch (condition) {
			case ConditionCode::Carry:    bitIsSet = getFlag(flags::c); break; // BCS/BCC
			case ConditionCode::Zero:     bitIsSet = getFlag(flags::z); break; // BEQ/BNQ
			case ConditionCode::Negative: bitIsSet = getFlag(flags::n); break; // BMI/BPL
			case ConditionCode::Overflow: bitIsSet = getFlag(flags::v); break; // BVS/BVC

			default:
				// this should be impossible
				abort();
		}

		// if the bit is set and we want to pass the condition if it's set (i.e. BCS, BEQ, BMI, BVS), OR
		// the bit is NOT set and we want to pass the condition if it's NOT set (i.e. BCC, BNQ, BPL, BVC),
		// then we go ahead with the branch and update the PC
		if ((bitIsSet && passConditionIfBitSet) || (!bitIsSet && !passConditionIfBitSet)) {
			// update PC
			PC = newPC;
		}
	}

	return 0;
};
