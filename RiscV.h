#pragma once

#include <sstream>

namespace RiscV {

	typedef int32_t INSTRUCTION;
	typedef int32_t ADDRESS;
	typedef int32_t WORD;
    typedef int16_t HALFWORD;
	typedef int8_t  BYTE;


	size_t const cRegCount = 32;
    size_t const cDataIncrement = 4;
    size_t const cMemDataSize = (1 << 16) / cDataIncrement;


    namespace UType {
        constexpr auto OP_LUI = 0b0110111;
        constexpr auto OP_AUIPC = 0b0010111;
    }

    namespace JType {
        constexpr auto OP_JAL = 0b1101111;
    }

    namespace RType {
        constexpr auto OP_TYPE_REGISTER = 0b0110011;

        // RV32I
        constexpr auto FUNC3_ADD = 0b000;
        constexpr auto FUNC7_ADD = 0b0000000;
        constexpr auto FUNC3_SUB = 0b000;
        constexpr auto FUNC7_SUB = 0b0100000;
        constexpr auto FUNC3_SLL = 0b001;
        constexpr auto FUNC7_SLL = 0b0000000;
        constexpr auto FUNC3_SLT = 0b010;
        constexpr auto FUNC7_SLT = 0b0000000;
        constexpr auto FUNC3_SLTU = 0b011;
        constexpr auto FUNC7_SLTU = 0b0000000;
        constexpr auto FUNC3_XOR = 0b100;
        constexpr auto FUNC7_XOR = 0b0000000;
        constexpr auto FUNC3_SRL = 0b101;
        constexpr auto FUNC7_SRL = 0b0000000;
        constexpr auto FUNC3_SRA = 0b101;
        constexpr auto FUNC7_SRA = 0b0100000;
        constexpr auto FUNC3_OR = 0b110;
        constexpr auto FUNC7_OR = 0b0000000;
        constexpr auto FUNC3_AND = 0b111;
        constexpr auto FUNC7_AND = 0b0000000;

        // RV32M
        constexpr auto FUNC3_MUL = 0b000;
        constexpr auto FUNC7_MUL = 0b0000001;
        constexpr auto FUNC3_MULH = 0b001;
        constexpr auto FUNC7_MULH = 0b0000001;
        constexpr auto FUNC3_MULHSU = 0b010;
        constexpr auto FUNC7_MULHSU = 0b0000001;
        constexpr auto FUNC3_MULHU = 0b011;
        constexpr auto FUNC7_MULHU = 0b0000001;
        constexpr auto FUNC3_DIV = 0b100;
        constexpr auto FUNC7_DIV = 0b0000001;
        constexpr auto FUNC3_DIVU = 0b101;
        constexpr auto FUNC7_DIVU = 0b0000001;
        constexpr auto FUNC3_REM = 0b110;
        constexpr auto FUNC7_REM = 0b0000001;
        constexpr auto FUNC3_REMU = 0b111;
        constexpr auto FUNC7_REMU = 0b0000001;
    }

    namespace IType {
        constexpr auto OP_TYPE_LOAD = 0b0000011;
        constexpr auto FUNC3_LB = 0b000;
        constexpr auto FUNC3_LH = 0b001;
        constexpr auto FUNC3_LW = 0b010;
        constexpr auto FUNC3_LBU = 0b100;
        constexpr auto FUNC3_LHU = 0b101;

        constexpr auto OP_TYPE_IMMEDIATE = 0b0010011;
        constexpr auto FUNC3_ADDI = 0b000;
        constexpr auto FUNC3_SLTI = 0b010;
        constexpr auto FUNC3_SLTIU = 0b011;
        constexpr auto FUNC3_XORI = 0b100;
        constexpr auto FUNC3_ORI = 0b110;
        constexpr auto FUNC3_ANDI = 0b111;
        constexpr auto FUNC3_SLLI = 0b001;
        constexpr auto FUNC6_SLLI = 0b000000;
        constexpr auto FUNC3_SRLI = 0b101;
        constexpr auto FUNC6_SRLI = 0b000000;
        constexpr auto FUNC3_SRAI = 0b101;
        constexpr auto FUNC6_SRAI = 0b010000;

        constexpr auto OP_JALR = 0b1100111;
        constexpr auto FUNC3_JALR = 0b000;

        // currently not needed and implemented, but exist in RV32I
        constexpr auto OP_TYPE_FENCE = 0b0001111;   // for I/O
        constexpr auto OP_TYPE_E = 0b1110011;       // for exceptions
        constexpr auto OP_TYPE_CSR = 0b1110011;     // for controls and status register
    }

    namespace SType {
        constexpr auto OP_TYPE_STORE = 0b0100011;
        constexpr auto FUNC3_SB = 0b000;
        constexpr auto FUNC3_SH = 0b001;
        constexpr auto FUNC3_SW = 0b010;
    }

    namespace BType {
        constexpr auto OP_TYPE_BRANCH = 0b1100011;
        constexpr auto FUNC3_BEQ = 0b000;
        constexpr auto FUNC3_BNEQ = 0b001;
        constexpr auto FUNC3_BLT = 0b100;
        constexpr auto FUNC3_BGE = 0b101;
        constexpr auto FUNC3_BLTU = 0b110;
        constexpr auto FUNC3_BGEU = 0b111;
    }


    namespace PType {
        constexpr auto OP_TYPE_PRINT = 0b1111111;
        constexpr auto FUNC3_INT = 0b0000000;
        constexpr auto FUNC3_STRING = 0b0000001;

        constexpr auto OP_TYPE_SLEEP = 0b1111110;
    }

    static std::stringstream disasm;
    void Disassemble(WORD machinecode);

	BYTE MaskOpcode(INSTRUCTION instruction);
	BYTE MaskFunct3(INSTRUCTION instruction);
	BYTE MaskFunct7(INSTRUCTION instruction);
    BYTE MaskRs1(INSTRUCTION instruction);
    BYTE MaskRs2(INSTRUCTION instruction);
    BYTE MaskRd(INSTRUCTION instruction);
}

