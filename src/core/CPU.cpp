#include <blaze/CPU.hpp>

const std::unordered_map<Blaze::Byte, Blaze::Cycles(Blaze::CPU::*)()> Blaze::CPU::SINGLE_BYTE_INSTRUCTIONS {
	{ 0x40, &CPU::executeRTI },
	{ 0x60, &CPU::executeRTS },
	{ 0x08, &CPU::executePHP },
	{ 0x28, &CPU::executePLP },
	{ 0x48, &CPU::executePHA },
	{ 0x68, &CPU::executePLA },
	{ 0x88, &CPU::executeDEY },
	{ 0xa8, &CPU::executeTAY },
	{ 0xc8, &CPU::executeINY },
	{ 0xe8, &CPU::executeINX },
	{ 0x18, &CPU::executeCLC },
	{ 0x38, &CPU::executeSEC },
	{ 0x58, &CPU::executeCLI },
	{ 0x78, &CPU::executeSEI },
	{ 0x98, &CPU::executeTYA },
	{ 0xb8, &CPU::executeCLV },
	{ 0xd8, &CPU::executeCLD },
	{ 0xf8, &CPU::executeSED },
	{ 0x8a, &CPU::executeTXA },
	{ 0x9a, &CPU::executeTXS },
	{ 0xaa, &CPU::executeTAX },
	{ 0xba, &CPU::executeTSX },
	{ 0xca, &CPU::executeDEX },
	{ 0xea, &CPU::executeNOP },
	{ 0x5a, &CPU::executePHY },
	{ 0x7a, &CPU::executePLY },
	{ 0xda, &CPU::executePHX },
	{ 0xfa, &CPU::executePLX },
	{ 0x0b, &CPU::executePHD },
	{ 0x2b, &CPU::executePLD },
	{ 0x4b, &CPU::executePHK },
	{ 0x6b, &CPU::executeRTL },
	{ 0x8b, &CPU::executePHB },
	{ 0xab, &CPU::executePLB },
	{ 0xcb, &CPU::executeWAI },
	{ 0xeb, &CPU::executeXBA },
	{ 0x1b, &CPU::executeTCS },
	{ 0x3b, &CPU::executeTSC },
	{ 0x5b, &CPU::executeTCD },
	{ 0x7b, &CPU::executeTDC },
	{ 0x9b, &CPU::executeTXY },
	{ 0xbb, &CPU::executeTYX },
	{ 0xdb, &CPU::executeSTP },
	{ 0xfb, &CPU::executeXCE },
	{ 0x42, &CPU::executeWDM },
};

const std::unordered_map<Blaze::Byte, std::pair<Blaze::Cycles(Blaze::CPU::*)(Blaze::CPU::AddressingMode), Blaze::CPU::AddressingMode>> Blaze::CPU::SINGLE_BYTE_INSTRUCTIONS_WITH_MODE {
	{ 0x1a, { &CPU::executeINC, AddressingMode::Accumulator } },
	{ 0x3a, { &CPU::executeDEC, AddressingMode::Accumulator } },
};

const std::unordered_map<Blaze::Byte, std::pair<Blaze::Byte, Blaze::Cycles(Blaze::CPU::*)()>> Blaze::CPU::SPECIAL_MULTI_BYTE_INSTRUCTIONS {
	{ 0x00, { 2, &CPU::executeBRK } }, // requires 2 bytes, but the 2nd byte is unused
	{ 0x02, { 2, &CPU::executeCOP } },
	{ 0x22, { 4, &CPU::executeJSL } },
	{ 0x62, { 3, &CPU::executePER } },
	{ 0x82, { 3, &CPU::executeBRL } },
	{ 0xc2, { 2, &CPU::executeREP } },
	{ 0xe2, { 2, &CPU::executeSEP } },
	{ 0x44, { 3, &CPU::executeMVP } },
	{ 0x54, { 3, &CPU::executeMVN } },
	{ 0xd4, { 2, &CPU::executePEI } },
	{ 0xf4, { 3, &CPU::executePEA } },
	{ 0xdc, { 3, &CPU::executeJML } },
};

const std::unordered_map<Blaze::Byte, std::tuple<Blaze::Byte, Blaze::Cycles(Blaze::CPU::*)(Blaze::CPU::AddressingMode), Blaze::CPU::AddressingMode>> Blaze::CPU::SPECIAL_MULTI_BYTE_INSTRUCTIONS_WITH_MODE {
	{ 0x20, { 3, &CPU::executeJSR, AddressingMode::Absolute } },
	{ 0x89, { 2, &CPU::executeBIT, AddressingMode::Immediate } },
	{ 0x14, { 2, &CPU::executeTRB, AddressingMode::Direct } },
	{ 0x1c, { 3, &CPU::executeTRB, AddressingMode::Absolute } },
	{ 0x64, { 2, &CPU::executeSTZ, AddressingMode::Direct } },
	{ 0x9c, { 3, &CPU::executeSTZ, AddressingMode::Absolute } },
	{ 0x74, { 2, &CPU::executeSTZ, AddressingMode::DirectIndexedX } },
	{ 0x9e, { 3, &CPU::executeSTZ, AddressingMode::AbsoluteIndexedX } },
	{ 0x5c, { 4, &CPU::executeJMP, AddressingMode::AbsoluteLong } },
	{ 0xfc, { 3, &CPU::executeJSR, AddressingMode::AbsoluteIndexedIndirect } },
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

static constexpr uint16_t concat16(uint8_t hi, uint8_t lo) {
	return (static_cast<uint16_t>(hi) << 8) | lo;
};

static constexpr uint32_t concat24(uint8_t hi, uint16_t lo) {
	return (static_cast<uint32_t>(hi) << 16) | lo;
};

static constexpr uint32_t concat24(uint8_t hi, uint8_t mid, uint8_t lo) {
	return concat24(hi, concat16(mid, lo));
};
// NOLINTEND(readability-magic-numbers, readability-identifier-length)

void Blaze::CPU::reset(MemRam &memory) {

	PC = 0xFFFC; // need to load w/contents of reset vector
	DBR = PBR = 0x00;
	X = Y = 0x00;
	SP = 0x0100;

	setFlag(d, false);

	setFlag(m, true);
	setFlag(x, true);
	setFlag(i, true);
	setFlag(c, true);

	_memory = &memory;
	_memory->reset();
}

void Blaze::CPU::execute(ClockTicks cycles) {
	while(cycles > 0) {
		const Byte* instruction = &(*_memory)[PC];
		cycles--;

		Byte instrSize = 0;
		auto instrCycles = executeInstruction(instrSize);

		PC += instrSize;
	}
}

void Blaze::CPU::setFlag(CPU::flags flag, bool s) {
	if (s)
		P |= flag; // set flag
	else
		P &= ~flag; // clear flag
}

Blaze::Byte Blaze::CPU::load8(Byte bank, Word addressLow) const {
	// TODO: load from memory
	return 0;
};

Blaze::Word Blaze::CPU::load16(Byte bank, Word addressLow) const {
	// TODO: load from memory
	return 0;
};

Blaze::Address Blaze::CPU::load24(Byte bank, Word addressLow) const {
	// TODO: load from memory
	return 0;
};

Blaze::Address Blaze::CPU::decodeAddress(AddressingMode mode) const {
	const Byte* inst = currentInstruction();

	switch (mode) {
		case AddressingMode::Absolute:
			return concat24(DBR, inst[2], inst[1]);
		case AddressingMode::AbsoluteIndexedIndirect:
			return load16(0, concat16(inst[2], inst[1]) + X);
		case AddressingMode::AbsoluteIndexedX:
			return concat24(DBR, concat16(inst[2], inst[1]) + X);
		case AddressingMode::AbsoluteIndexedY:
			return concat24(DBR, concat16(inst[2], inst[1]) + Y);

		case AddressingMode::AbsoluteIndirect: {
			auto base = concat16(inst[2], inst[1]);
			if (inst[0] == /* JML */ 0xdc) {
				return load24(0, base);
			} else {
				return load16(0, base);
			}
		} break;

		case AddressingMode::AbsoluteLongIndexedX:
			return concat24(inst[3], inst[2], inst[1]) + X;
		case AddressingMode::AbsoluteLong:
			return concat24(inst[3], inst[2], inst[1]);
		case AddressingMode::DirectIndexedIndirect:
			return concat24(DBR, load16(0, DR + X + inst[1]));
		case AddressingMode::DirectIndexedX:
			return concat24(0, DR + X + inst[1]);
		case AddressingMode::DirectIndexedY:
			return concat24(0, DR + Y + inst[1]);
		case AddressingMode::DirectIndirectIndexed:
			return concat24(DBR, load16(0, DR + inst[1])) + Y;
		case AddressingMode::DirectIndirectLongIndexed:
			return load24(0, DR + inst[1]) + Y;
		case AddressingMode::DirectIndirectLong:
			return load24(0, DR + inst[1]);
		case AddressingMode::DirectIndirect:
			return concat24(DBR, load16(0, DR + inst[1]));
		case AddressingMode::Direct:
			return concat24(0, DR + inst[1]);
		case AddressingMode::ProgramCounterRelativeLong:
			// `PC + 3` since the PC used for the calculation is the address of the *next* instruction
			return static_cast<uint16_t>(static_cast<int16_t>(PC + 3) + static_cast<int16_t>(concat16(inst[2], inst[1])));
		case AddressingMode::ProgramCounterRelative:
			// ditto
			return static_cast<uint16_t>(static_cast<int16_t>(PC + 2) + static_cast<int8_t>(inst[1]));
		case AddressingMode::StackRelative:
			return concat24(0, SP + inst[1]);
		case AddressingMode::StackRelativeIndirectIndexed:
			return concat24(DBR, load16(0, SP + inst[1])) + Y;

		case AddressingMode::Accumulator:
		case AddressingMode::BlockMove:
		case AddressingMode::Immediate:
		case AddressingMode::Implied:
		case AddressingMode::Stack:
		default:
			return 0;
	}
};

// special thanks to https://llx.com/Neil/a2/opcodes.html for some wisdom on how intelligently decode the instructions
// (without having a giant switch statement)
Blaze::Cycles Blaze::CPU::executeInstruction(Byte& outInstructionSize) {
	const Byte* inst = currentInstruction();
	outInstructionSize = 0;

	// before doing any smart decoding, we first do some simple opcode comparisons.
	// there are some instructions that only require a single byte (their opcode).
	// then there are those instructions that require multiple bytes, but have no
	// clear pattern that can be used to decode them "intelligently".

	// try single-byte instructions first
	auto singleByteEntry = SINGLE_BYTE_INSTRUCTIONS.find(inst[0]);
	if (singleByteEntry != SINGLE_BYTE_INSTRUCTIONS.end()) {
		auto memberFunc = singleByteEntry->second;
		outInstructionSize = 1;
		return (this->*memberFunc)();
	}

	auto singleByteWithModeEntry = SINGLE_BYTE_INSTRUCTIONS_WITH_MODE.find(inst[0]);
	if (singleByteWithModeEntry != SINGLE_BYTE_INSTRUCTIONS_WITH_MODE.end()) {
		auto funcInfo = singleByteWithModeEntry->second;
		auto memberFunc = funcInfo.first;
		auto addrMode = funcInfo.second;
		outInstructionSize = 1;
		return (this->*memberFunc)(addrMode);
	}

	// now try special multi-byte instructions
	auto multiByteEntry = SPECIAL_MULTI_BYTE_INSTRUCTIONS.find(inst[0]);
	if (multiByteEntry != SPECIAL_MULTI_BYTE_INSTRUCTIONS.end()) {
		auto funcInfo = multiByteEntry->second;
		auto instrSize = funcInfo.first;
		auto memberFunc = funcInfo.second;
		outInstructionSize = instrSize;
		return (this->*memberFunc)();
	}

	auto multiByteWithModeEntry = SPECIAL_MULTI_BYTE_INSTRUCTIONS_WITH_MODE.find(inst[0]);
	if (multiByteWithModeEntry != SPECIAL_MULTI_BYTE_INSTRUCTIONS_WITH_MODE.end()) {
		auto funcInfo = multiByteWithModeEntry->second;
		auto instrSize = std::get<0>(funcInfo);
		auto memberFunc = std::get<1>(funcInfo);
		auto addrMode = std::get<2>(funcInfo);
		outInstructionSize = instrSize;
		return (this->*memberFunc)(addrMode);
	}

	// this is a super special case
	if (inst[0] == 0x80) {
		outInstructionSize = 2;
		return executeBRA(ConditionCode::NONE, false);
	}

	// for the rest of the opcodes, we decode them according to patterns

	auto groupSelect = opcodeGetGroupSelect(inst[0]);
	auto namespacedAddrMode = opcodeGetAddressingMode(inst[0]);
	auto namespacedOpcode = opcodeGetSubopcode(inst[0]);

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
				return invalidInstruction();
			}

			if (instructionSize == 0) {
				// there are only 2 cases where this happens: Immediate and Stack
				// Stack is impossible because it's not supported in this group,
				// so it must be Immediate. in this case, the instruction size depends
				// on the CPU flags.
				//
				// when the `m` flag is zero, it takes up 3 bytes instead of 2.
				instructionSize = (f.m == 0) ? 3 : 2;
			}

			outInstructionSize = instructionSize;

			switch (opcode) {
				case Group1Opcode::ORA: return executeORA(mode);
				case Group1Opcode::AND: return executeAND(mode);
				case Group1Opcode::EOR: return executeEOR(mode);
				case Group1Opcode::ADC: return executeADC(mode);
				case Group1Opcode::STA: return executeSTA(mode);
				case Group1Opcode::LDA: return executeLDA(mode);
				case Group1Opcode::CMP: return executeCMP(mode);
				case Group1Opcode::SBC: return executeSBC(mode);
			}
		} break;

		case 2: {
			// Group 2

			// if the address mode matches the special 65C02 address mode, we process it as a Group 1 instruction instead.
			if (namespacedAddrMode == GROUP2_65C02_ADDRESS_MODE) {
				outInstructionSize = 2;

				switch (static_cast<Group1Opcode>(namespacedOpcode)) {
					case Group1Opcode::ORA: return executeORA(AddressingMode::DirectIndirect);
					case Group1Opcode::AND: return executeAND(AddressingMode::DirectIndirect);
					case Group1Opcode::EOR: return executeEOR(AddressingMode::DirectIndirect);
					case Group1Opcode::ADC: return executeADC(AddressingMode::DirectIndirect);
					case Group1Opcode::STA: return executeSTA(AddressingMode::DirectIndirect);
					case Group1Opcode::LDA: return executeLDA(AddressingMode::DirectIndirect);
					case Group1Opcode::CMP: return executeCMP(AddressingMode::DirectIndirect);
					case Group1Opcode::SBC: return executeSBC(AddressingMode::DirectIndirect);
				}
			}

			AddressingMode mode = GROUP2_ADDRESS_MODE_MAP[namespacedAddrMode];
			auto instructionSize = instructionSizeWithAddressingMode(mode);
			auto opcode = static_cast<Group2Opcode>(namespacedOpcode);

			if (mode == AddressingMode::INVALID) {
				return invalidInstruction();
			}

			// in this group, not all instructions support all valid modes

			if (
				// only LDX supports immediate addressing
				(mode == AddressingMode::Immediate && opcode != Group2Opcode::LDX) ||
				// STX doesn't support Absolute Indexed X addressing
				(mode == AddressingMode::AbsoluteIndexedX && opcode == Group2Opcode::STX)
			) {
				return invalidInstruction();
			}

			if (mode == AddressingMode::Accumulator) {
				switch (opcode) {
					// STX, LDX, DEC, and INC don't support Accumulator addressing
					case Group2Opcode::STX:
					case Group2Opcode::LDX:
					case Group2Opcode::DEC:
					case Group2Opcode::INC:
						return invalidInstruction();

					default:
						break;
				}
			}

			if (instructionSize == 0) {
				// same as for Group 1 instructions
				instructionSize = (f.m == 0) ? 3 : 2;
			}

			outInstructionSize = instructionSize;

			switch (opcode) {
				case Group2Opcode::ASL: return executeASL(mode);
				case Group2Opcode::ROL: return executeROL(mode);
				case Group2Opcode::LSR: return executeLSR(mode);
				case Group2Opcode::ROR: return executeROR(mode);
				case Group2Opcode::STX: return executeSTX(mode);
				case Group2Opcode::LDX: return executeLDX(mode);
				case Group2Opcode::DEC: return executeDEC(mode);
				case Group2Opcode::INC: return executeINC(mode);
			}
		} break;

		case 0: {
			// Group 3

			// if the address mode matches the special condition address mode, we process it as a branch instruction with a condition.
			if (namespacedAddrMode == GROUP3_CONDITION_ADDRESS_MODE) {
				outInstructionSize = 1;
				return executeBRA(static_cast<ConditionCode>(namespacedOpcode >> 1), (namespacedOpcode & 0x01) != 0);
			}

			AddressingMode mode = GROUP3_ADDRESS_MODE_MAP[namespacedAddrMode];
			auto instructionSize = instructionSizeWithAddressingMode(mode);
			auto opcode = static_cast<Group3Opcode>(namespacedOpcode);

			if (mode == AddressingMode::INVALID) {
				return invalidInstruction();
			}

			// in this group, not all instructions support all valid modes

			if (
				// JMP and JMPIndirect don't support Direct addressing
				(mode == AddressingMode::Direct && (opcode == Group3Opcode::JMP || opcode == Group3Opcode::JMPIndirect)) ||
				// only STY, LDY, and BIT support Direct Indexed X addressing
				(mode == AddressingMode::DirectIndexedX && opcode != Group3Opcode::STY && opcode != Group3Opcode::LDY && opcode != Group3Opcode::BIT) ||
				// only LDY and BIT support Absolute Indexed X addressing
				(mode == AddressingMode::AbsoluteIndexedX && opcode != Group3Opcode::LDY && opcode != Group3Opcode::BIT) ||
				// only LDY, CPY, and CPX support Immediate addressing
				(mode == AddressingMode::Immediate && opcode != Group3Opcode::LDY && opcode != Group3Opcode::CPY && opcode != Group3Opcode::CPX)
			) {
				return invalidInstruction();
			}

			if (instructionSize == 0) {
				// same as for Group 1 instructions
				instructionSize = (f.m == 0) ? 3 : 2;
			}

			outInstructionSize = instructionSize;

			switch (opcode) {
				case Group3Opcode::TSB:         return executeTSB(mode);
				case Group3Opcode::BIT:         return executeBIT(mode);
				case Group3Opcode::JMP:         return executeJMP(mode);
				case Group3Opcode::STY:         return executeSTY(mode);
				case Group3Opcode::LDY:         return executeLDY(mode);
				case Group3Opcode::CPY:         return executeCPY(mode);
				case Group3Opcode::CPX:         return executeCPX(mode);
				case Group3Opcode::JMPIndirect:
					// this is a special case, because we have to take the given
					// addressing mode and use the indirect version of it
					switch (mode) {
						case AddressingMode::Absolute:         return executeJMP(AddressingMode::AbsoluteIndirect);
						case AddressingMode::AbsoluteIndexedX: return executeJMP(AddressingMode::AbsoluteIndexedIndirect);

						default:
							return invalidInstruction();
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
				instructionSize = (f.m == 0) ? 3 : 2;
			}

			outInstructionSize = instructionSize;

			switch (opcode) {
				case Group1Opcode::ORA: return executeORA(mode);
				case Group1Opcode::AND: return executeAND(mode);
				case Group1Opcode::EOR: return executeEOR(mode);
				case Group1Opcode::ADC: return executeADC(mode);
				case Group1Opcode::STA: return executeSTA(mode);
				case Group1Opcode::LDA: return executeLDA(mode);
				case Group1Opcode::CMP: return executeCMP(mode);
				case Group1Opcode::SBC: return executeSBC(mode);
			}
		} break;

		default:
			return invalidInstruction();
	}
};

Blaze::Cycles Blaze::CPU::invalidInstruction() {
	// TODO
	return 0;
};

const Blaze::Byte* Blaze::CPU::currentInstruction() const {
	return &(*_memory)[PC];
};

Blaze::Cycles Blaze::CPU::executeBRK() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeBRL() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCLC() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCLD() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCLI() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCLV() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCOP() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeDEX() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeDEY() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeINX() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeINY() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeJML() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeJSL() {
	// TODO
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
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePEA() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePEI() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePER() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHA() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHB() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHD() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHK() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHP() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHX() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePHY() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLA() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLB() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLD() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLP() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLX() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executePLY() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeREP() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeRTI() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeRTL() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeRTS() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSEC() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSED() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSEI() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSEP() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSTP() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTAX() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTAY() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTCD() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTCS() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTDC() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTSC() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTSX() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTXA() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTXS() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTXY() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTYA() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTYX() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeWAI() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeWDM() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeXBA() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeXCE() {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeADC(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeAND(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeASL(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeBIT(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCMP(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCPX(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeCPY(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeDEC(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeEOR(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeINC(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeJMP(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeJSR(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeLDA(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeLDX(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeLDY(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeLSR(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeORA(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeROL(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeROR(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSBC(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSTA(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSTX(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSTY(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeSTZ(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTRB(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeTSB(AddressingMode mode) {
	// TODO
	return 0;
};

Blaze::Cycles Blaze::CPU::executeBRA(ConditionCode condition, bool passConditionIfBitSet) {
	// TODO
	return 0;
};
