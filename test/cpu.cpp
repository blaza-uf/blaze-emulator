#include <blaze/Bus.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include <sstream>
#include <blaze/util.hpp>

#include "bus-mock.hpp"

using namespace Blaze;
using Instruction = Blaze::CPU::Instruction;
using Opcode = Blaze::CPU::Opcode;
using AddressingMode = Blaze::CPU::AddressingMode;
using ConditionCode = Blaze::CPU::ConditionCode;

static constexpr std::array<Instruction, 256> OPCODE_INFO {
	/* 00 */ Instruction(Opcode::BRK, 2, 0, AddressingMode::Stack),
	/* 01 */ Instruction(Opcode::ORA, 2, 0, AddressingMode::DirectIndexedIndirect),
	/* 02 */ Instruction(Opcode::COP, 2, 0, AddressingMode::Stack),
	/* 03 */ Instruction(Opcode::ORA, 2, 0, AddressingMode::StackRelative),
	/* 04 */ Instruction(Opcode::TSB, 2, 0, AddressingMode::Direct),
	/* 05 */ Instruction(Opcode::ORA, 2, 0, AddressingMode::Direct),
	/* 06 */ Instruction(Opcode::ASL, 2, 0, AddressingMode::Direct),
	/* 07 */ Instruction(Opcode::ORA, 2, 0, AddressingMode::DirectIndirectLong),
	/* 08 */ Instruction(Opcode::PHP, 1, 0, AddressingMode::Stack),
	/* 09 */ Instruction(Opcode::ORA, 2, 0, AddressingMode::Immediate),
	/* 0A */ Instruction(Opcode::ASL, 1, 0, AddressingMode::Accumulator),
	/* 0B */ Instruction(Opcode::PHD, 1, 0, AddressingMode::Stack),
	/* 0C */ Instruction(Opcode::TSB, 3, 0, AddressingMode::Absolute),
	/* 0D */ Instruction(Opcode::ORA, 3, 0, AddressingMode::Absolute),
	/* 0E */ Instruction(Opcode::ASL, 3, 0, AddressingMode::Absolute),
	/* 0F */ Instruction(Opcode::ORA, 4, 0, AddressingMode::AbsoluteLong),
	/* 10 */ Instruction(/* BPL */ Opcode::BRA, 2, 0, ConditionCode::Negative, false),
	/* 11 */ Instruction(Opcode::ORA, 2, 0, AddressingMode::DirectIndirectIndexed),
	/* 12 */ Instruction(Opcode::ORA, 2, 0, AddressingMode::DirectIndirect),
	/* 13 */ Instruction(Opcode::ORA, 2, 0, AddressingMode::StackRelativeIndirectIndexed),
	/* 14 */ Instruction(Opcode::TRB, 2, 0, AddressingMode::Direct),
	/* 15 */ Instruction(Opcode::ORA, 2, 0, AddressingMode::DirectIndexedX),
	/* 16 */ Instruction(Opcode::ASL, 2, 0, AddressingMode::DirectIndexedX),
	/* 17 */ Instruction(Opcode::ORA, 2, 0, AddressingMode::DirectIndirectLongIndexed),
	/* 18 */ Instruction(Opcode::CLC, 1, 0, AddressingMode::Implied),
	/* 19 */ Instruction(Opcode::ORA, 3, 0, AddressingMode::AbsoluteIndexedY),
	/* 1A */ Instruction(Opcode::INC, 1, 0, AddressingMode::Accumulator),
	/* 1B */ Instruction(Opcode::TCS, 1, 0, AddressingMode::Implied),
	/* 1C */ Instruction(Opcode::TRB, 3, 0, AddressingMode::Absolute),
	/* 1D */ Instruction(Opcode::ORA, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 1E */ Instruction(Opcode::ASL, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 1F */ Instruction(Opcode::ORA, 4, 0, AddressingMode::AbsoluteLongIndexedX),
	/* 20 */ Instruction(Opcode::JSR, 3, 0, AddressingMode::Absolute),
	/* 21 */ Instruction(Opcode::AND, 2, 0, AddressingMode::DirectIndexedIndirect),
	/* 22 */ Instruction(Opcode::JSL, 4, 0, AddressingMode::AbsoluteLong),
	/* 23 */ Instruction(Opcode::AND, 2, 0, AddressingMode::StackRelative),
	/* 24 */ Instruction(Opcode::BIT, 2, 0, AddressingMode::Direct),
	/* 25 */ Instruction(Opcode::AND, 2, 0, AddressingMode::Direct),
	/* 26 */ Instruction(Opcode::ROL, 2, 0, AddressingMode::Direct),
	/* 27 */ Instruction(Opcode::AND, 2, 0, AddressingMode::DirectIndirectLong),
	/* 28 */ Instruction(Opcode::PLP, 1, 0, AddressingMode::Stack),
	/* 29 */ Instruction(Opcode::AND, 2, 0, AddressingMode::Immediate),
	/* 2A */ Instruction(Opcode::ROL, 1, 0, AddressingMode::Accumulator),
	/* 2B */ Instruction(Opcode::PLD, 1, 0, AddressingMode::Stack),
	/* 2C */ Instruction(Opcode::BIT, 3, 0, AddressingMode::Absolute),
	/* 2D */ Instruction(Opcode::AND, 3, 0, AddressingMode::Absolute),
	/* 2E */ Instruction(Opcode::ROL, 3, 0, AddressingMode::Absolute),
	/* 2F */ Instruction(Opcode::AND, 4, 0, AddressingMode::AbsoluteLong),
	/* 30 */ Instruction(/* BMI */ Opcode::BRA, 2, 0, ConditionCode::Negative, true),
	/* 31 */ Instruction(Opcode::AND, 2, 0, AddressingMode::DirectIndirectIndexed),
	/* 32 */ Instruction(Opcode::AND, 2, 0, AddressingMode::DirectIndirect),
	/* 33 */ Instruction(Opcode::AND, 2, 0, AddressingMode::StackRelativeIndirectIndexed),
	/* 34 */ Instruction(Opcode::BIT, 2, 0, AddressingMode::DirectIndexedX),
	/* 35 */ Instruction(Opcode::AND, 2, 0, AddressingMode::DirectIndexedX),
	/* 36 */ Instruction(Opcode::ROL, 2, 0, AddressingMode::DirectIndexedX),
	/* 37 */ Instruction(Opcode::AND, 2, 0, AddressingMode::DirectIndirectLongIndexed),
	/* 38 */ Instruction(Opcode::SEC, 1, 0, AddressingMode::Implied),
	/* 39 */ Instruction(Opcode::AND, 3, 0, AddressingMode::AbsoluteIndexedY),
	/* 3A */ Instruction(Opcode::DEC, 1, 0, AddressingMode::Accumulator),
	/* 3B */ Instruction(Opcode::TSC, 1, 0, AddressingMode::Implied),
	/* 3C */ Instruction(Opcode::BIT, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 3D */ Instruction(Opcode::AND, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 3E */ Instruction(Opcode::ROL, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 3F */ Instruction(Opcode::AND, 4, 0, AddressingMode::AbsoluteLongIndexedX),
	/* 40 */ Instruction(Opcode::RTI, 1, 0, AddressingMode::Stack),
	/* 41 */ Instruction(Opcode::EOR, 2, 0, AddressingMode::DirectIndexedIndirect),
	/* 42 */ Instruction(Opcode::WDM, 2, 0, AddressingMode::Implied),
	/* 43 */ Instruction(Opcode::EOR, 2, 0, AddressingMode::StackRelative),
	/* 44 */ Instruction(Opcode::MVP, 3, 0, AddressingMode::BlockMove),
	/* 45 */ Instruction(Opcode::EOR, 2, 0, AddressingMode::Direct),
	/* 46 */ Instruction(Opcode::LSR, 2, 0, AddressingMode::Direct),
	/* 47 */ Instruction(Opcode::EOR, 2, 0, AddressingMode::DirectIndirectLong),
	/* 48 */ Instruction(Opcode::PHA, 1, 0, AddressingMode::Stack),
	/* 49 */ Instruction(Opcode::EOR, 2, 0, AddressingMode::Immediate),
	/* 4A */ Instruction(Opcode::LSR, 1, 0, AddressingMode::Accumulator),
	/* 4B */ Instruction(Opcode::PHK, 1, 0, AddressingMode::Stack),
	/* 4C */ Instruction(Opcode::JMP, 3, 0, AddressingMode::Absolute),
	/* 4D */ Instruction(Opcode::EOR, 3, 0, AddressingMode::Absolute),
	/* 4E */ Instruction(Opcode::LSR, 3, 0, AddressingMode::Absolute),
	/* 4F */ Instruction(Opcode::EOR, 4, 0, AddressingMode::AbsoluteLong),
	/* 50 */ Instruction(/* BVC */ Opcode::BRA, 2, 0, ConditionCode::Overflow, false),
	/* 51 */ Instruction(Opcode::EOR, 2, 0, AddressingMode::DirectIndirectIndexed),
	/* 52 */ Instruction(Opcode::EOR, 2, 0, AddressingMode::DirectIndirect),
	/* 53 */ Instruction(Opcode::EOR, 2, 0, AddressingMode::StackRelativeIndirectIndexed),
	/* 54 */ Instruction(Opcode::MVN, 3, 0, AddressingMode::BlockMove),
	/* 55 */ Instruction(Opcode::EOR, 2, 0, AddressingMode::DirectIndexedX),
	/* 56 */ Instruction(Opcode::LSR, 2, 0, AddressingMode::DirectIndexedX),
	/* 57 */ Instruction(Opcode::EOR, 2, 0, AddressingMode::DirectIndirectLongIndexed),
	/* 58 */ Instruction(Opcode::CLI, 1, 0, AddressingMode::Implied),
	/* 59 */ Instruction(Opcode::EOR, 3, 0, AddressingMode::AbsoluteIndexedY),
	/* 5A */ Instruction(Opcode::PHY, 1, 0, AddressingMode::Stack),
	/* 5B */ Instruction(Opcode::TCD, 1, 0, AddressingMode::Implied),
	/* 5C */ Instruction(Opcode::JMP, 4, 0, AddressingMode::AbsoluteLong),
	/* 5D */ Instruction(Opcode::EOR, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 5E */ Instruction(Opcode::LSR, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 5F */ Instruction(Opcode::EOR, 4, 0, AddressingMode::AbsoluteLongIndexedX),
	/* 60 */ Instruction(Opcode::RTS, 1, 0, AddressingMode::Stack),
	/* 61 */ Instruction(Opcode::ADC, 2, 0, AddressingMode::DirectIndexedIndirect),
	/* 62 */ Instruction(Opcode::PER, 3, 0, AddressingMode::Stack),
	/* 63 */ Instruction(Opcode::ADC, 2, 0, AddressingMode::StackRelative),
	/* 64 */ Instruction(Opcode::STZ, 2, 0, AddressingMode::Direct),
	/* 65 */ Instruction(Opcode::ADC, 2, 0, AddressingMode::Direct),
	/* 66 */ Instruction(Opcode::ROR, 2, 0, AddressingMode::Direct),
	/* 67 */ Instruction(Opcode::ADC, 2, 0, AddressingMode::DirectIndirectLong),
	/* 68 */ Instruction(Opcode::PLA, 1, 0, AddressingMode::Stack),
	/* 69 */ Instruction(Opcode::ADC, 2, 0, AddressingMode::Immediate),
	/* 6A */ Instruction(Opcode::ROR, 1, 0, AddressingMode::Accumulator),
	/* 6B */ Instruction(Opcode::RTL, 1, 0, AddressingMode::Stack),
	/* 6C */ Instruction(Opcode::JMP, 3, 0, AddressingMode::AbsoluteIndirect),
	/* 6D */ Instruction(Opcode::ADC, 3, 0, AddressingMode::Absolute),
	/* 6E */ Instruction(Opcode::ROR, 3, 0, AddressingMode::Absolute),
	/* 6F */ Instruction(Opcode::ADC, 4, 0, AddressingMode::AbsoluteLong),
	/* 70 */ Instruction(/* BVS */ Opcode::BRA, 2, 0, ConditionCode::Overflow, true),
	/* 71 */ Instruction(Opcode::ADC, 2, 0, AddressingMode::DirectIndirectIndexed),
	/* 72 */ Instruction(Opcode::ADC, 2, 0, AddressingMode::DirectIndirect),
	/* 73 */ Instruction(Opcode::ADC, 2, 0, AddressingMode::StackRelativeIndirectIndexed),
	/* 74 */ Instruction(Opcode::STZ, 2, 0, AddressingMode::DirectIndexedX),
	/* 75 */ Instruction(Opcode::ADC, 2, 0, AddressingMode::DirectIndexedX),
	/* 76 */ Instruction(Opcode::ROR, 2, 0, AddressingMode::DirectIndexedX),
	/* 77 */ Instruction(Opcode::ADC, 2, 0, AddressingMode::DirectIndirectLongIndexed),
	/* 78 */ Instruction(Opcode::SEI, 1, 0, AddressingMode::Implied),
	/* 79 */ Instruction(Opcode::ADC, 3, 0, AddressingMode::AbsoluteIndexedY),
	/* 7A */ Instruction(Opcode::PLY, 1, 0, AddressingMode::Stack),
	/* 7B */ Instruction(Opcode::TDC, 1, 0, AddressingMode::Implied),
	/* 7C */ Instruction(Opcode::JMP, 3, 0, AddressingMode::AbsoluteIndexedIndirect),
	/* 7D */ Instruction(Opcode::ADC, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 7E */ Instruction(Opcode::ROR, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 7F */ Instruction(Opcode::ADC, 4, 0, AddressingMode::AbsoluteLongIndexedX),
	/* 80 */ Instruction(Opcode::BRA, 2, 0, ConditionCode::NONE, false),
	/* 81 */ Instruction(Opcode::STA, 2, 0, AddressingMode::DirectIndexedIndirect),
	/* 82 */ Instruction(Opcode::BRL, 3, 0, ConditionCode::NONE, false),
	/* 83 */ Instruction(Opcode::STA, 2, 0, AddressingMode::StackRelative),
	/* 84 */ Instruction(Opcode::STY, 2, 0, AddressingMode::Direct),
	/* 85 */ Instruction(Opcode::STA, 2, 0, AddressingMode::Direct),
	/* 86 */ Instruction(Opcode::STX, 2, 0, AddressingMode::Direct),
	/* 87 */ Instruction(Opcode::STA, 2, 0, AddressingMode::DirectIndirectLong),
	/* 88 */ Instruction(Opcode::DEY, 1, 0, AddressingMode::Implied),
	/* 89 */ Instruction(Opcode::BIT, 2, 0, AddressingMode::Immediate),
	/* 8A */ Instruction(Opcode::TXA, 1, 0, AddressingMode::Implied),
	/* 8B */ Instruction(Opcode::PHB, 1, 0, AddressingMode::Stack),
	/* 8C */ Instruction(Opcode::STY, 3, 0, AddressingMode::Absolute),
	/* 8D */ Instruction(Opcode::STA, 3, 0, AddressingMode::Absolute),
	/* 8E */ Instruction(Opcode::STX, 3, 0, AddressingMode::Absolute),
	/* 8F */ Instruction(Opcode::STA, 4, 0, AddressingMode::AbsoluteLong),
	/* 90 */ Instruction(/* BCC */ Opcode::BRA, 2, 0, ConditionCode::Carry, false),
	/* 91 */ Instruction(Opcode::STA, 2, 0, AddressingMode::DirectIndirectIndexed),
	/* 92 */ Instruction(Opcode::STA, 2, 0, AddressingMode::DirectIndirect),
	/* 93 */ Instruction(Opcode::STA, 2, 0, AddressingMode::StackRelativeIndirectIndexed),
	/* 94 */ Instruction(Opcode::STY, 2, 0, AddressingMode::DirectIndexedX),
	/* 95 */ Instruction(Opcode::STA, 2, 0, AddressingMode::DirectIndexedX),
	/* 96 */ Instruction(Opcode::STX, 2, 0, AddressingMode::DirectIndexedX /* actually AddressingMode::DirectIndexedY */),
	/* 97 */ Instruction(Opcode::STA, 2, 0, AddressingMode::DirectIndirectLongIndexed),
	/* 98 */ Instruction(Opcode::TYA, 1, 0, AddressingMode::Implied),
	/* 99 */ Instruction(Opcode::STA, 3, 0, AddressingMode::AbsoluteIndexedY),
	/* 9A */ Instruction(Opcode::TXS, 1, 0, AddressingMode::Implied),
	/* 9B */ Instruction(Opcode::TXY, 1, 0, AddressingMode::Implied),
	/* 9C */ Instruction(Opcode::STZ, 3, 0, AddressingMode::Absolute),
	/* 9D */ Instruction(Opcode::STA, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 9E */ Instruction(Opcode::STZ, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* 9F */ Instruction(Opcode::STA, 4, 0, AddressingMode::AbsoluteLongIndexedX),
	/* A0 */ Instruction(Opcode::LDY, 2, 0, AddressingMode::Immediate),
	/* A1 */ Instruction(Opcode::LDA, 2, 0, AddressingMode::DirectIndexedIndirect),
	/* A2 */ Instruction(Opcode::LDX, 2, 0, AddressingMode::Immediate),
	/* A3 */ Instruction(Opcode::LDA, 2, 0, AddressingMode::StackRelative),
	/* A4 */ Instruction(Opcode::LDY, 2, 0, AddressingMode::Direct),
	/* A5 */ Instruction(Opcode::LDA, 2, 0, AddressingMode::Direct),
	/* A6 */ Instruction(Opcode::LDX, 2, 0, AddressingMode::Direct),
	/* A7 */ Instruction(Opcode::LDA, 2, 0, AddressingMode::DirectIndirectLong),
	/* A8 */ Instruction(Opcode::TAY, 1, 0, AddressingMode::Implied),
	/* A9 */ Instruction(Opcode::LDA, 2, 0, AddressingMode::Immediate),
	/* AA */ Instruction(Opcode::TAX, 1, 0, AddressingMode::Implied),
	/* AB */ Instruction(Opcode::PLB, 1, 0, AddressingMode::Stack),
	/* AC */ Instruction(Opcode::LDY, 3, 0, AddressingMode::Absolute),
	/* AD */ Instruction(Opcode::LDA, 3, 0, AddressingMode::Absolute),
	/* AE */ Instruction(Opcode::LDX, 3, 0, AddressingMode::Absolute),
	/* AF */ Instruction(Opcode::LDA, 4, 0, AddressingMode::AbsoluteLong),
	/* B0 */ Instruction(/* BCS */ Opcode::BRA, 2, 0, ConditionCode::Carry, true),
	/* B1 */ Instruction(Opcode::LDA, 2, 0, AddressingMode::DirectIndirectIndexed),
	/* B2 */ Instruction(Opcode::LDA, 2, 0, AddressingMode::DirectIndirect),
	/* B3 */ Instruction(Opcode::LDA, 2, 0, AddressingMode::StackRelativeIndirectIndexed),
	/* B4 */ Instruction(Opcode::LDY, 2, 0, AddressingMode::DirectIndexedX),
	/* B5 */ Instruction(Opcode::LDA, 2, 0, AddressingMode::DirectIndexedX),
	/* B6 */ Instruction(Opcode::LDX, 2, 0, AddressingMode::DirectIndexedX /* actually AddressingMode::DirectIndexedY */),
	/* B7 */ Instruction(Opcode::LDA, 2, 0, AddressingMode::DirectIndirectLongIndexed),
	/* B8 */ Instruction(Opcode::CLV, 1, 0, AddressingMode::Implied),
	/* B9 */ Instruction(Opcode::LDA, 3, 0, AddressingMode::AbsoluteIndexedY),
	/* BA */ Instruction(Opcode::TSX, 1, 0, AddressingMode::Implied),
	/* BB */ Instruction(Opcode::TYX, 1, 0, AddressingMode::Implied),
	/* BC */ Instruction(Opcode::LDY, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* BD */ Instruction(Opcode::LDA, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* BE */ Instruction(Opcode::LDX, 3, 0, AddressingMode::AbsoluteIndexedX /* actually AddressingMode::AbsoluteIndexedY */),
	/* BF */ Instruction(Opcode::LDA, 4, 0, AddressingMode::AbsoluteLongIndexedX),
	/* C0 */ Instruction(Opcode::CPY, 2, 0, AddressingMode::Immediate),
	/* C1 */ Instruction(Opcode::CMP, 2, 0, AddressingMode::DirectIndexedIndirect),
	/* C2 */ Instruction(Opcode::REP, 2, 0, AddressingMode::Immediate),
	/* C3 */ Instruction(Opcode::CMP, 2, 0, AddressingMode::StackRelative),
	/* C4 */ Instruction(Opcode::CPY, 2, 0, AddressingMode::Direct),
	/* C5 */ Instruction(Opcode::CMP, 2, 0, AddressingMode::Direct),
	/* C6 */ Instruction(Opcode::DEC, 2, 0, AddressingMode::Direct),
	/* C7 */ Instruction(Opcode::CMP, 2, 0, AddressingMode::DirectIndirectLong),
	/* C8 */ Instruction(Opcode::INY, 1, 0, AddressingMode::Implied),
	/* C9 */ Instruction(Opcode::CMP, 2, 0, AddressingMode::Immediate),
	/* CA */ Instruction(Opcode::DEX, 1, 0, AddressingMode::Implied),
	/* CB */ Instruction(Opcode::WAI, 1, 0, AddressingMode::Implied),
	/* CC */ Instruction(Opcode::CPY, 3, 0, AddressingMode::Absolute),
	/* CD */ Instruction(Opcode::CMP, 3, 0, AddressingMode::Absolute),
	/* CE */ Instruction(Opcode::DEC, 3, 0, AddressingMode::Absolute),
	/* CF */ Instruction(Opcode::CMP, 4, 0, AddressingMode::AbsoluteLong),
	/* D0 */ Instruction(/* BNE */ Opcode::BRA, 2, 0, ConditionCode::Zero, false),
	/* D1 */ Instruction(Opcode::CMP, 2, 0, AddressingMode::DirectIndirectIndexed),
	/* D2 */ Instruction(Opcode::CMP, 2, 0, AddressingMode::DirectIndirect),
	/* D3 */ Instruction(Opcode::CMP, 2, 0, AddressingMode::StackRelativeIndirectIndexed),
	/* D4 */ Instruction(Opcode::PEI, 2, 0, AddressingMode::Stack),
	/* D5 */ Instruction(Opcode::CMP, 2, 0, AddressingMode::DirectIndexedX),
	/* D6 */ Instruction(Opcode::DEC, 2, 0, AddressingMode::DirectIndexedX),
	/* D7 */ Instruction(Opcode::CMP, 2, 0, AddressingMode::DirectIndirectLongIndexed),
	/* D8 */ Instruction(Opcode::CLD, 1, 0, AddressingMode::Implied),
	/* D9 */ Instruction(Opcode::CMP, 3, 0, AddressingMode::AbsoluteIndexedY),
	/* DA */ Instruction(Opcode::PHX, 1, 0, AddressingMode::Stack),
	/* DB */ Instruction(Opcode::STP, 1, 0, AddressingMode::Implied),
	/* DC */ Instruction(Opcode::JML, 3, 0, AddressingMode::AbsoluteIndirect),
	/* DD */ Instruction(Opcode::CMP, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* DE */ Instruction(Opcode::DEC, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* DF */ Instruction(Opcode::CMP, 4, 0, AddressingMode::AbsoluteLongIndexedX),
	/* E0 */ Instruction(Opcode::CPX, 2, 0, AddressingMode::Immediate),
	/* E1 */ Instruction(Opcode::SBC, 2, 0, AddressingMode::DirectIndexedIndirect),
	/* E2 */ Instruction(Opcode::SEP, 2, 0, AddressingMode::Immediate),
	/* E3 */ Instruction(Opcode::SBC, 2, 0, AddressingMode::StackRelative),
	/* E4 */ Instruction(Opcode::CPX, 2, 0, AddressingMode::Direct),
	/* E5 */ Instruction(Opcode::SBC, 2, 0, AddressingMode::Direct),
	/* E6 */ Instruction(Opcode::INC, 2, 0, AddressingMode::Direct),
	/* E7 */ Instruction(Opcode::SBC, 2, 0, AddressingMode::DirectIndirectLong),
	/* E8 */ Instruction(Opcode::INX, 1, 0, AddressingMode::Implied),
	/* E9 */ Instruction(Opcode::SBC, 2, 0, AddressingMode::Immediate),
	/* EA */ Instruction(Opcode::NOP, 1, 0, AddressingMode::Implied),
	/* EB */ Instruction(Opcode::XBA, 1, 0, AddressingMode::Implied),
	/* EC */ Instruction(Opcode::CPX, 3, 0, AddressingMode::Absolute),
	/* ED */ Instruction(Opcode::SBC, 3, 0, AddressingMode::Absolute),
	/* EE */ Instruction(Opcode::INC, 3, 0, AddressingMode::Absolute),
	/* EF */ Instruction(Opcode::SBC, 4, 0, AddressingMode::AbsoluteLong),
	/* F0 */ Instruction(/* BEQ */ Opcode::BRA, 2, 0, ConditionCode::Zero, true),
	/* F1 */ Instruction(Opcode::SBC, 2, 0, AddressingMode::DirectIndirectIndexed),
	/* F2 */ Instruction(Opcode::SBC, 2, 0, AddressingMode::DirectIndirect),
	/* F3 */ Instruction(Opcode::SBC, 2, 0, AddressingMode::StackRelativeIndirectIndexed),
	/* F4 */ Instruction(Opcode::PEA, 3, 0, AddressingMode::Stack),
	/* F5 */ Instruction(Opcode::SBC, 2, 0, AddressingMode::DirectIndexedX),
	/* F6 */ Instruction(Opcode::INC, 2, 0, AddressingMode::DirectIndexedX),
	/* F7 */ Instruction(Opcode::SBC, 2, 0, AddressingMode::DirectIndirectLongIndexed),
	/* F8 */ Instruction(Opcode::SED, 1, 0, AddressingMode::Implied),
	/* F9 */ Instruction(Opcode::SBC, 3, 0, AddressingMode::AbsoluteIndexedY),
	/* FA */ Instruction(Opcode::PLX, 1, 0, AddressingMode::Stack),
	/* FB */ Instruction(Opcode::XCE, 1, 0, AddressingMode::Implied),
	/* FC */ Instruction(Opcode::JSR, 3, 0, AddressingMode::AbsoluteIndexedIndirect),
	/* FD */ Instruction(Opcode::SBC, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* FE */ Instruction(Opcode::INC, 3, 0, AddressingMode::AbsoluteIndexedX),
	/* FF */ Instruction(Opcode::SBC, 4, 0, AddressingMode::AbsoluteLongIndexedX),
};

static void testInstruction(CPU::Opcode opcode, AddressingMode addressingMode, std::function<void(CPU&, std::vector<Testing::BusAccess>&)> addExpectedBusAccesses, std::function<void(CPU&)> setup, std::function<void(CPU&)> test) {
	static constexpr Address INSTRUCTION_ADDRESS = 0x8000;
	static constexpr Word STACK_POINTER = 0x01f0;

	CPU cpu;
	Byte rawOpcode = 0;
	Address pcWhileExecuting = INSTRUCTION_ADDRESS;
	std::vector<Testing::BusAccess> busAccesses;

	do {
		if (OPCODE_INFO[rawOpcode].opcode == opcode && OPCODE_INFO[rawOpcode].addressingMode == addressingMode) {
			break;
		}
		++rawOpcode;
	} while (rawOpcode != 0);

	cpu.PC = INSTRUCTION_ADDRESS;
	cpu.executingPC = cpu.PC;
	cpu.SP = STACK_POINTER;

	cpu.PC += OPCODE_INFO[rawOpcode].size;

	addExpectedBusAccesses(cpu, busAccesses);

	Testing::PreconfiguredBus mockBus(true, busAccesses);
	cpu.bus = &mockBus;

	setup(cpu);

	cpu.executeInstruction(OPCODE_INFO[rawOpcode]);

	mockBus.finalize();

	test(cpu);
};

static void testInstructionWithOperand(CPU::Opcode opcode, Byte operandBitSize, Address operand, const std::initializer_list<AddressingMode>& addressingModes, std::function<void(CPU&, std::vector<Testing::BusAccess>&)> addExpectedBusAccesses, std::function<void(CPU&)> setup, std::function<void(CPU&)> test) {
	static constexpr Address INSTRUCTION_ADDRESS = 0x8000;
	static constexpr Address OPERAND_ADDRESS = 0x8100;
	static constexpr Address INDIRECT_OPERAND_ADDRESS = 0x8200;
	static constexpr Address DIRECT_OPERAND_ADDRESS = 0x0080;
	static constexpr Address LONG_OPERAND_ADDRESS = 0x202020;
	static constexpr Word INDEX_OFFSET = 0x0008;
	static constexpr Address BRANCH_ADDRESS = 0x7f7f;
	static constexpr Address LONG_BRANCH_ADDRESS = 0x4089;
	static constexpr Word STACK_POINTER = 0x01f0;
	static constexpr Word STACK_POINTER_OFFSET = 0x08;

	for (const auto& addressingMode: addressingModes) {
		SECTION(CPU::ADDRESSING_MODE_NAMES[static_cast<uint8_t>(addressingMode)]) {
			CPU cpu;
			Byte rawOpcode = 0;
			Address pcWhileExecuting = INSTRUCTION_ADDRESS;
			std::vector<Testing::BusAccess> busAccesses;

			do {
				if (OPCODE_INFO[rawOpcode].opcode == opcode && OPCODE_INFO[rawOpcode].addressingMode == addressingMode) {
					break;
				}
				++rawOpcode;
			} while (rawOpcode != 0);

			switch (addressingMode) {
				case CPU::AddressingMode::Absolute:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 16, OPERAND_ADDRESS},
						{false, OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::AbsoluteIndexedIndirect:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 16, INDIRECT_OPERAND_ADDRESS - INDEX_OFFSET},
						{false, INDIRECT_OPERAND_ADDRESS, 16, OPERAND_ADDRESS},
						{false, OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::AbsoluteIndexedX:
				case CPU::AddressingMode::AbsoluteIndexedY:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 16, OPERAND_ADDRESS - INDEX_OFFSET},
						{false, OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::AbsoluteIndirect:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 16, INDIRECT_OPERAND_ADDRESS},
						{false, INDIRECT_OPERAND_ADDRESS, static_cast<Byte>((opcode == Opcode::JML) ? 24 : 16), OPERAND_ADDRESS},
						{false, OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::AbsoluteLongIndexedX:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 24, LONG_OPERAND_ADDRESS - INDEX_OFFSET},
						{false, LONG_OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::AbsoluteLong:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 24, LONG_OPERAND_ADDRESS},
						{false, LONG_OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::DirectIndexedIndirect:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 8, DIRECT_OPERAND_ADDRESS - INDEX_OFFSET},
						{false, DIRECT_OPERAND_ADDRESS, 16, OPERAND_ADDRESS},
						{false, OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::DirectIndexedX:
				case CPU::AddressingMode::DirectIndexedY:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 8, DIRECT_OPERAND_ADDRESS - INDEX_OFFSET},
						{false, DIRECT_OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::DirectIndirectIndexed:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 8, DIRECT_OPERAND_ADDRESS},
						{false, DIRECT_OPERAND_ADDRESS, 16, OPERAND_ADDRESS - INDEX_OFFSET},
						{false, OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::DirectIndirectLongIndexed:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 8, DIRECT_OPERAND_ADDRESS},
						{false, DIRECT_OPERAND_ADDRESS, 24, LONG_OPERAND_ADDRESS - INDEX_OFFSET},
						{false, LONG_OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::DirectIndirectLong:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 8, DIRECT_OPERAND_ADDRESS},
						{false, DIRECT_OPERAND_ADDRESS, 24, LONG_OPERAND_ADDRESS},
						{false, LONG_OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::DirectIndirect:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 8, DIRECT_OPERAND_ADDRESS},
						{false, DIRECT_OPERAND_ADDRESS, 16, OPERAND_ADDRESS},
						{false, OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::Direct:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 8, DIRECT_OPERAND_ADDRESS},
						{false, DIRECT_OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::Immediate:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::ProgramCounterRelativeLong:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 16, lo16(LONG_BRANCH_ADDRESS - pcWhileExecuting)},
					};
					break;

				case CPU::AddressingMode::ProgramCounterRelative:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 8, lo8(BRANCH_ADDRESS - pcWhileExecuting)},
					};
					break;

				case CPU::AddressingMode::StackRelative:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 8, STACK_POINTER_OFFSET},
						{false, STACK_POINTER + STACK_POINTER_OFFSET, operandBitSize, operand},
					};
					break;

				case CPU::AddressingMode::StackRelativeIndirectIndexed:
					busAccesses = {
						{false, INSTRUCTION_ADDRESS + 1, 8, STACK_POINTER_OFFSET},
						{false, STACK_POINTER + STACK_POINTER_OFFSET, 16, OPERAND_ADDRESS - INDEX_OFFSET},
						{false, OPERAND_ADDRESS, operandBitSize, operand},
					};
					break;

				default:
					break;
			}

			cpu.PC = INSTRUCTION_ADDRESS;
			cpu.executingPC = cpu.PC;
			cpu.SP = STACK_POINTER;

			cpu.PC += OPCODE_INFO[rawOpcode].size;

			switch (addressingMode) {
				case CPU::AddressingMode::AbsoluteIndexedIndirect:
				case CPU::AddressingMode::AbsoluteIndexedX:
				case CPU::AddressingMode::AbsoluteLongIndexedX:
				case CPU::AddressingMode::DirectIndexedIndirect:
				case CPU::AddressingMode::DirectIndexedX:
					cpu.X = INDEX_OFFSET;

				case CPU::AddressingMode::AbsoluteIndexedY:
				case CPU::AddressingMode::DirectIndexedY:
				case CPU::AddressingMode::DirectIndirectIndexed:
				case CPU::AddressingMode::DirectIndirectLongIndexed:
				case CPU::AddressingMode::StackRelativeIndirectIndexed:
					cpu.Y = INDEX_OFFSET;
					break;

				case CPU::AddressingMode::BlockMove:
					// this requires a special case; we don't handle this in this function
					throw std::runtime_error("Invalid addressing mode (block move) given to testInstructionWithOperand");

				default:
					break;
			}

			addExpectedBusAccesses(cpu, busAccesses);

			Testing::PreconfiguredBus mockBus(true, busAccesses);
			cpu.bus = &mockBus;

			setup(cpu);

			cpu.executeInstruction(OPCODE_INFO[rawOpcode]);

			mockBus.finalize();

			test(cpu);
		}
	}
};

static void noopTestStep(CPU& cpu) {};
static void noopAddBusAccesses(CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {};

TEST_CASE("Instruction decoding", "[cpu]") {
	Byte opcodeByte;
	Instruction expectedInfo;
	std::tie(opcodeByte, expectedInfo) = GENERATE(map([](size_t const& index) { return std::make_pair(index, OPCODE_INFO[index]); }, range(0, 256)));

	std::stringstream asHex;
	asHex << std::hex << static_cast<uint32_t>(opcodeByte);
	std::string resultHex(asHex.str());

	DYNAMIC_SECTION("Decode instruction 0x" << resultHex) {
		auto decodedInfo = CPU::decodeInstruction(opcodeByte, true);

		// the static_casts below are for proper integer output for test assertions

		REQUIRE(static_cast<uint32_t>(decodedInfo.opcode) == static_cast<uint32_t>(expectedInfo.opcode));
		REQUIRE(static_cast<uint32_t>(decodedInfo.size) == static_cast<uint32_t>(expectedInfo.size));

		if (expectedInfo.addressingMode == AddressingMode::Implied || expectedInfo.addressingMode == AddressingMode::Stack) {
			// we don't decode this info; we just leave the addressing mode invalid
			REQUIRE(static_cast<uint32_t>(decodedInfo.addressingMode) == static_cast<uint32_t>(AddressingMode::INVALID));
		} else {
			REQUIRE(static_cast<uint32_t>(decodedInfo.addressingMode) == static_cast<uint32_t>(expectedInfo.addressingMode));
		}

		REQUIRE(static_cast<uint32_t>(decodedInfo.condition) == static_cast<uint32_t>(expectedInfo.condition));
		REQUIRE(decodedInfo.passConditionIfBitSet == expectedInfo.passConditionIfBitSet);
	}
}

TEST_CASE("ADC", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto hasCarry = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto fullResult = static_cast<Address>(lhs) + static_cast<Address>(rhs) + (hasCarry ? 1 : 0);
	auto result = static_cast<Word>(fullResult & (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff));
	bool resultIsZero = result == 0;
	bool resultIsNegative = msb(result, memoryAndAccumulatorAre8Bit);
	bool resultHasOverflow = msb(lhs, memoryAndAccumulatorAre8Bit) == msb(rhs, memoryAndAccumulatorAre8Bit) && msb(lhs, memoryAndAccumulatorAre8Bit) != msb(result, memoryAndAccumulatorAre8Bit);
	bool resultHasCarry = fullResult != result;

	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; " << (hasCarry ? "" : "no ") << "carry") {
		testInstructionWithOperand(Opcode::ADC, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::DirectIndexedIndirect,
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
				AddressingMode::DirectIndirectIndexed,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedY,
				AddressingMode::AbsoluteIndexedX,
				AddressingMode::StackRelative,
				AddressingMode::DirectIndirect,
				AddressingMode::DirectIndirectLong,
				AddressingMode::AbsoluteLong,
				AddressingMode::StackRelativeIndirectIndexed,
				AddressingMode::DirectIndirectLongIndexed,
				AddressingMode::AbsoluteLongIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
				cpu.setFlag(CPU::flags::c, hasCarry);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.load() == result);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
				REQUIRE(cpu.getFlag(CPU::flags::v) == resultHasOverflow);
				REQUIRE(cpu.getFlag(CPU::flags::c) == resultHasCarry);
			}
		);
	}
}

TEST_CASE("PHA", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1)));

	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PHA, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(true, initialSP - (memoryAndAccumulatorAre8Bit ? 0 : 1), memoryAndAccumulatorAre8Bit ? 8 : 16, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(val);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				// the store is already verified by the bus access list
				//
				// we just need to check the stack pointer here
				REQUIRE(cpu.SP == initialSP - (memoryAndAccumulatorAre8Bit ? 1 : 2));
			}
		);
	}
}

TEST_CASE("PLA", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1)));
	
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PLA, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(false, initialSP + 1, memoryAndAccumulatorAre8Bit ? 8 : 16, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.load() == val);
				REQUIRE(cpu.SP == initialSP + (memoryAndAccumulatorAre8Bit ? 1 : 2));
			}
		);
	}
}

TEST_CASE("PHD", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (usingEmulatorMode ? 0xff : 0xffff) + 1)));
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PHD, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(true, initialSP - (usingEmulatorMode ? 0 : 1), usingEmulatorMode ? 8 : 16, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.DR = val;
			},
			/*test=*/[&](CPU& cpu) {
	
				REQUIRE(cpu.SP == initialSP - (usingEmulatorMode ? 1 : 2));
			}
		);
	}
}

TEST_CASE("PLD", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (usingEmulatorMode ? 0xff : 0xffff) + 1)));
	bool resultIsZero = val == 0;
	bool resultIsNegative = msb(val, usingEmulatorMode);
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PLD, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(false, initialSP + 1, usingEmulatorMode ? 8 : 16, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.DR == val);
				REQUIRE(cpu.SP == initialSP + (usingEmulatorMode ? 1 : 2));
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("PHX", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));

	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PHX, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(true, initialSP - (indexRegistersAre8Bit ? 0 : 1), indexRegistersAre8Bit ? 8 : 16, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.X.forceStoreFull(val);
				cpu.setFlag(CPU::flags::x,indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.SP == initialSP - (indexRegistersAre8Bit ? 1 : 2));
			}
		);
	}
}

TEST_CASE("PLX", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
	bool resultIsZero = val == 0;
	bool resultIsNegative = msb(val, indexRegistersAre8Bit);
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PLX, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(false, initialSP + 1,indexRegistersAre8Bit ? 8 : 16, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.X.load() == val);
				REQUIRE(cpu.SP == initialSP + (indexRegistersAre8Bit ? 1 : 2));
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("PHY", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));

	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PHY, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(true, initialSP - (indexRegistersAre8Bit ? 0 : 1), indexRegistersAre8Bit ? 8 : 16, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.Y.forceStoreFull(val);
				cpu.setFlag(CPU::flags::x,indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.SP == initialSP - (indexRegistersAre8Bit ? 1 : 2));
			}
		);
	}
}

TEST_CASE("PLY", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
	bool resultIsZero = val == 0;
	bool resultIsNegative = msb(val, indexRegistersAre8Bit);
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PLY, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(false, initialSP + 1,indexRegistersAre8Bit ? 8 : 16, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.Y.load() == val);
				REQUIRE(cpu.SP == initialSP + (indexRegistersAre8Bit ? 1 : 2));
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("PHB", "[cpu][instruction]") {
	auto val = GENERATE_COPY(take(1, random<Address>(0, 0xff + 1)));

	DYNAMIC_SECTION(8 << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PHB, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(true, initialSP, 8, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.DBR = val;
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.SP == initialSP - 1);
			}
		);
	}
}

TEST_CASE("PLB", "[cpu][instruction]") {
	auto val = GENERATE_COPY(take(1, random<Address>(0, 0xff + 1)));
	bool resultIsZero = val == 0;
	bool resultIsNegative = msb8(val);
	DYNAMIC_SECTION(8 << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PLB, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(false, initialSP + 1, 8, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.DBR == val);
				REQUIRE(cpu.SP == initialSP + 1);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("PHK", "[cpu][instruction]") {
	auto val = GENERATE_COPY(take(1, random<Address>(0, 0xff + 1)));

	DYNAMIC_SECTION(8 << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PHK, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(true, initialSP, 8, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.PBR = val;
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.SP == initialSP - 1);
			}
		);
	}
}

TEST_CASE("PHP", "[cpu][instruction]") {
	auto val = GENERATE_COPY(take(1, random<Address>(0, 0xff + 1)));

	DYNAMIC_SECTION(8 << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PHP, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(true, initialSP, 8, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.P = val;
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.SP == initialSP - 1);
			}
		);
	}
}

TEST_CASE("PLP", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, 0xff + 1)));
	DYNAMIC_SECTION(8 << "-bit") {
		Word initialSP = 0;

		testInstruction(Opcode::PLP, AddressingMode::Stack,
			/*addExpectedBusAccesses=*/[&](CPU& cpu, std::vector<Testing::BusAccess>& busAccesses) {
				initialSP = cpu.SP;

				busAccesses.emplace_back(false, initialSP + 1, 8, val);
			},
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.setFlag(CPU::flags::m, usingEmulatorMode);
				cpu.setFlag(CPU::flags::x, usingEmulatorMode);
			},
			/*test=*/[&](CPU& cpu) {
				auto expectedVal = val | (usingEmulatorMode ? (CPU::flags::m | CPU::flags::x) : 0);
				REQUIRE(cpu.P == expectedVal);
				REQUIRE(cpu.SP == initialSP + 1);
			}
		);
	}
}

TEST_CASE("TCS", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1)));
			
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TCS, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(val);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.SP == cpu.A.forceLoadFull());
			}
		);
	}
}

TEST_CASE("TSC", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1)));
			
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TSC, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.SP = val;
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.forceLoadFull() == cpu.SP);
			}
		);
	}
}

TEST_CASE("TAX", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
			
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TAX, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(val);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.X.load() == cpu.A.load());
			}
		);
	}
}

TEST_CASE("TXA", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1)));
	
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TXA, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.X.forceStoreFull(val);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.load() == cpu.X.load());
			}
		);
	}
}

TEST_CASE("TAY", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
		
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TAY, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(val);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.Y.load() == cpu.A.load());
			}
		);
	}
}

TEST_CASE("TYA", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1)));
		
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TYA, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.Y.forceStoreFull(val);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.load() == cpu.Y.load());
			}
		);
	}
}

TEST_CASE("TXY", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
			
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TXY, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.X.forceStoreFull(val);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.Y.load() == cpu.X.load());
			}
		);
	}
}

TEST_CASE("TYX", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
			
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TYX, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.Y.forceStoreFull(val);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.X.load() == cpu.Y.load());
			}
		);
	}
}

TEST_CASE("TSX", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
			
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TSX, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.SP = val;
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.X.load() == cpu.SP);
			}
		);
	}
}

TEST_CASE("TXS", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (usingEmulatorMode ? 0xff : 0xffff) + 1)));
			
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TXS, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.X.forceStoreFull(val);
			},
			/*test=*/[&](CPU& cpu) {
				auto expectedVal = cpu.X.load() | (usingEmulatorMode ? 0x0100 : 0);
				REQUIRE(cpu.SP == expectedVal);
			}
		);
	}
}

TEST_CASE("TDC", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1)));
		
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TDC, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.DR = val;
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.forceLoadFull() == cpu.DR);
			}
		);
	}
}

TEST_CASE("TCD", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (usingEmulatorMode ? 0xff : 0xffff) + 1)));
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::TCD, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.A.forceStoreFull(val);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.DR == cpu.A.forceLoadFull());	
			}
		);
	}
}

TEST_CASE("AND", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs & rhs;
	bool resultIsZero = result == 0;
	bool resultIsNegative = msb(result, memoryAndAccumulatorAre8Bit);

	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::AND, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::DirectIndexedIndirect,
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
				AddressingMode::DirectIndirectIndexed,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedY,
				AddressingMode::AbsoluteIndexedX,
				AddressingMode::StackRelative,
				AddressingMode::DirectIndirect,
				AddressingMode::DirectIndirectLong,
				AddressingMode::AbsoluteLong,
				AddressingMode::StackRelativeIndirectIndexed,
				AddressingMode::DirectIndirectLongIndexed,
				AddressingMode::AbsoluteLongIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.load() == result);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("ORA", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs | rhs;
	bool resultIsZero = result == 0;
	bool resultIsNegative = msb(result, memoryAndAccumulatorAre8Bit);

	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::ORA, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::DirectIndexedIndirect,
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
				AddressingMode::DirectIndirectIndexed,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedY,
				AddressingMode::AbsoluteIndexedX,
				AddressingMode::StackRelative,
				AddressingMode::DirectIndirect,
				AddressingMode::DirectIndirectLong,
				AddressingMode::AbsoluteLong,
				AddressingMode::StackRelativeIndirectIndexed,
				AddressingMode::DirectIndirectLongIndexed,
				AddressingMode::AbsoluteLongIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.load() == result);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("EOR", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs ^ rhs;
	bool resultIsZero = result == 0;
	bool resultIsNegative = msb(result, memoryAndAccumulatorAre8Bit);

	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::EOR, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::DirectIndexedIndirect,
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
				AddressingMode::DirectIndirectIndexed,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedY,
				AddressingMode::AbsoluteIndexedX,
				AddressingMode::StackRelative,
				AddressingMode::DirectIndirect,
				AddressingMode::DirectIndirectLong,
				AddressingMode::AbsoluteLong,
				AddressingMode::StackRelativeIndirectIndexed,
				AddressingMode::DirectIndirectLongIndexed,
				AddressingMode::AbsoluteLongIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.load() == result);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("CMP", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs - rhs;
	bool resultIsZero = lhs == rhs;
	bool resultIsNegative = msb(result, memoryAndAccumulatorAre8Bit);
	bool resultHasCarry = lhs >= rhs;
	
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::CMP, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::DirectIndexedIndirect,
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
				AddressingMode::DirectIndirectIndexed,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedY,
				AddressingMode::AbsoluteIndexedX,
				AddressingMode::StackRelative,
				AddressingMode::DirectIndirect,
				AddressingMode::DirectIndirectLong,
				AddressingMode::AbsoluteLong,
				AddressingMode::StackRelativeIndirectIndexed,
				AddressingMode::DirectIndirectLongIndexed,
				AddressingMode::AbsoluteLongIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(result >= 0);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
				REQUIRE(cpu.getFlag(CPU::flags::c) == resultHasCarry);
			}
		);
	}
}

TEST_CASE("LDA", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	bool resultIsZero = lhs == 0;
	bool resultIsNegative = msb(lhs, memoryAndAccumulatorAre8Bit);

	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::LDA, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::DirectIndexedIndirect,
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
				AddressingMode::DirectIndirectIndexed,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedY,
				AddressingMode::AbsoluteIndexedX,
				AddressingMode::StackRelative,
				AddressingMode::DirectIndirect,
				AddressingMode::DirectIndirectLong,
				AddressingMode::AbsoluteLong,
				AddressingMode::StackRelativeIndirectIndexed,
				AddressingMode::DirectIndirectLongIndexed,
				AddressingMode::AbsoluteLongIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.load() == rhs);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("LDX", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1))));
	bool resultIsZero = lhs == 0;
	bool resultIsNegative = msb(lhs, indexRegistersAre8Bit);

	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::LDX, indexRegistersAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.X.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.X.load() == rhs);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("LDY", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1))));
	bool resultIsZero = lhs == 0;
	bool resultIsNegative = msb(lhs, indexRegistersAre8Bit);

	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::LDY, indexRegistersAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.Y.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.Y.load() == rhs);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("CPX", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs - rhs;
	bool resultIsZero = lhs == rhs;
	bool resultIsNegative = msb(lhs, indexRegistersAre8Bit);
	bool resultHasCarry = lhs >= rhs;

	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::CPX, indexRegistersAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.X.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(result >= 0);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
				REQUIRE(cpu.getFlag(CPU::flags::c) == resultHasCarry);
			}
		);
	}
}

TEST_CASE("CPY", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs - rhs;
	bool resultIsZero = lhs == rhs;
	bool resultIsNegative = msb(lhs, indexRegistersAre8Bit);
	bool resultHasCarry = lhs >= rhs;

	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::CPY, indexRegistersAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.Y.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(result >= 0);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
				REQUIRE(cpu.getFlag(CPU::flags::c) == resultHasCarry);
			}
		);
	}
}

TEST_CASE("BIT", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs & rhs;
	bool resultIsZero = result == 0;
	bool resultIsNegative = msb(rhs, memoryAndAccumulatorAre8Bit);
	bool resultHasOverflow = memoryAndAccumulatorAre8Bit ? ((rhs & (1u << 6)) != 0) : ((rhs & (1u << 14)) != 0);
	
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::BIT, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::Direct,
				AddressingMode::Immediate,
				AddressingMode::Absolute,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
				REQUIRE(cpu.getFlag(CPU::flags::v) == resultHasOverflow);
			}
		);
	}
}

TEST_CASE("INC", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs + 1;
	bool resultIsZero = result == 0;
	bool resultIsNegative = msb(result, memoryAndAccumulatorAre8Bit);
	
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::INC, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::Direct,
				AddressingMode::Accumulator,
				AddressingMode::Absolute,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.load() == result);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("DEC", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs - 1;
	bool resultIsZero = result == 0;
	bool resultIsNegative = msb(result, memoryAndAccumulatorAre8Bit);
	
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::DEC, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::Direct,
				AddressingMode::Accumulator,
				AddressingMode::Absolute,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);

			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.A.load() == result);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
				REQUIRE(cpu.getFlag(CPU::flags::n) == resultIsNegative);
			}
		);
	}
}

TEST_CASE("INX", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
	auto result = val + 1;	
	
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::INX, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.X.forceStoreFull(val);
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.X.load() == result);
			}
		);
	}
}

TEST_CASE("INY", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
	auto result = val + 1;	
	
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::INY, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.Y.forceStoreFull(val);
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.Y.load() == result);
			}
		);
	}
}

TEST_CASE("DEX", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
	auto result = val - 1;	
	
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::DEX, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.X.forceStoreFull(val);
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.X.load() == result);
			}
		);
	}
}

TEST_CASE("DEY", "[cpu][instruction]") {
	auto indexRegistersAre8Bit = GENERATE(false, true);
	auto val = GENERATE_COPY(take(1, random<Address>(0, (indexRegistersAre8Bit ? 0xff : 0xffff) + 1)));
	auto result = val - 1;	
	
	DYNAMIC_SECTION((indexRegistersAre8Bit ? 8 : 16) << "-bit") {
		testInstruction(Opcode::DEY, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.Y.forceStoreFull(val);
				cpu.setFlag(CPU::flags::x, indexRegistersAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.Y.load() == result);
			}
		);
	}
}

TEST_CASE("CLV", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::CLV, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.setFlag(CPU::flags::v, false);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.getFlag(CPU::flags::v) == false);
			}
		);
	}
}

TEST_CASE("CLC", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::CLC, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.setFlag(CPU::flags::c, false);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.getFlag(CPU::flags::c) == false);
			}
		);
	}
}

TEST_CASE("CLI", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::CLI, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.setFlag(CPU::flags::i, false);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.getFlag(CPU::flags::i) == false);
			}
		);
	}
}

TEST_CASE("CLD", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::CLD, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.setFlag(CPU::flags::d, false);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.getFlag(CPU::flags::d) == false);
			}
		);
	}
}

TEST_CASE("SEC", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::SEC, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.setFlag(CPU::flags::c, true);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.getFlag(CPU::flags::c) == true);
			}
		);
	}
}

TEST_CASE("SED", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::SED, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.setFlag(CPU::flags::d, true);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.getFlag(CPU::flags::d) == true);
			}
		);
	}
}

TEST_CASE("SEI", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::SEI, AddressingMode::Implied,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.setFlag(CPU::flags::i, true);
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.getFlag(CPU::flags::i) == true);
			}
		);
	}
}

TEST_CASE("REP", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	auto valP = GENERATE_COPY(take(1, random<Address>(0, 0xff + 1)));
	auto val = GENERATE_COPY(take(1, random<Address>(0, 0xff + 1)));
	auto result = valP & ~val;
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::REP, AddressingMode::Immediate,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.P = valP;
				cpu.setFlag(CPU::flags::m, usingEmulatorMode);
				cpu.setFlag(CPU::flags::x, usingEmulatorMode);
			},
			/*test=*/[&](CPU& cpu) {
				auto expectedVal = result | (usingEmulatorMode ? (CPU::flags::m | CPU::flags::x) : 0);
				REQUIRE(cpu.P == expectedVal);
			}
		);
	}
}

TEST_CASE("SEP", "[cpu][instruction]") {
	auto usingEmulatorMode = GENERATE(false, true);
	auto valP = GENERATE_COPY(take(1, random<Address>(0, 0xff + 1)));
	auto val = GENERATE_COPY(take(1, random<Address>(0, 0xff + 1)));
	auto result = valP | val;
	
	DYNAMIC_SECTION((usingEmulatorMode ? 8 : 16) << "-bit") {
		testInstruction(Opcode::SEP, AddressingMode::Immediate,
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = usingEmulatorMode ? 1 : 0;
				cpu.P = valP;
			},
			/*test=*/[&](CPU& cpu) {
				REQUIRE(cpu.P == result);
			}
		);
	}
}

TEST_CASE("TRB", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs & rhs;
	bool resultIsZero = result == 0;
	auto finalResult = rhs & ~lhs;
	
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::TRB, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::Direct,
				AddressingMode::Absolute,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
				
			},
			/*test=*/[&](CPU& cpu) {
				auto expectedVal = rhs & ~cpu.A.load();
				REQUIRE(expectedVal == finalResult);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
			}
		);
	}
}

TEST_CASE("TSB", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto lhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	auto result = lhs & rhs;
	bool resultIsZero = result == 0;
	auto finalResult = rhs | lhs;
	
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::TSB, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::Direct,
				AddressingMode::Absolute,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.A.forceStoreFull(lhs);
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
				
			},
			/*test=*/[&](CPU& cpu) {
				auto expectedVal = rhs | cpu.A.load();
				REQUIRE(expectedVal == finalResult);
				REQUIRE(cpu.getFlag(CPU::flags::z) == resultIsZero);
			}
		);
	}
}

TEST_CASE("STZ", "[cpu][instruction]") {
	auto memoryAndAccumulatorAre8Bit = GENERATE(false, true);
	auto rhs = static_cast<Word>(GENERATE_COPY(take(1, random<Address>(0, (memoryAndAccumulatorAre8Bit ? 0xff : 0xffff) + 1))));
	
	DYNAMIC_SECTION((memoryAndAccumulatorAre8Bit ? 8 : 16) << "-bit; ") {
		testInstructionWithOperand(Opcode::STZ, memoryAndAccumulatorAre8Bit ? 8 : 16, rhs,
			{
				AddressingMode::Direct,
				AddressingMode::Absolute,
				AddressingMode::DirectIndexedX,
				AddressingMode::AbsoluteIndexedX,
			},
			/*addExpectedBusAccesses=*/noopAddBusAccesses,
			/*setup=*/[&](CPU& cpu) {
				cpu.e = 0;
				cpu.setFlag(CPU::flags::m, memoryAndAccumulatorAre8Bit);
			},
			/*test=*/[&](CPU& cpu) {
			REQUIRE(rhs == 0);	
			}
		);
	}
}
