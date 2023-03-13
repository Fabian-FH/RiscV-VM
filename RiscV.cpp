#include "RiscV.h"

#include <string>
#include <ostream>

void RiscV::Disassemble(WORD machinecode)
{
    WORD opcode = (machinecode & 0x7F);
    //char instruction[10] = "";
    std::string instruction = "";

    switch (opcode)
    {
    case RType::OP_TYPE_REGISTER: {
        WORD rd = (machinecode & 0xf80) >> 7;
        WORD f3 = (machinecode & 0x7000) >> 12;
        WORD rs1 = (machinecode & 0xf8000) >> 15;
        WORD rs2 = (machinecode & 0x1f00000) >> 20;
        WORD f7 = (machinecode & 0xfe000000) >> 25;
        //wprintf(L"disassembled R-Type instruction:\n");

        if (f3 == RType::FUNC3_ADD && f7 == RType::FUNC7_ADD) instruction = "add"; //strcpy(instruction, "add");
        else if (f3 == RType::FUNC3_SUB && f7 == RType::FUNC7_SUB) instruction = "sub"; //strcpy(instruction, "sub");
        else if (f3 == RType::FUNC3_SLL && f7 == RType::FUNC7_SLL) instruction = "sll"; //strcpy(instruction, "sll");
        else if (f3 == RType::FUNC3_SLT && f7 == RType::FUNC7_SLT) instruction = "slt"; //strcpy(instruction, "slt");
        else if (f3 == RType::FUNC3_SLTU && f7 == RType::FUNC7_SLTU) instruction = "sltu"; //strcpy(instruction, "sltu");
        else if (f3 == RType::FUNC3_XOR && f7 == RType::FUNC7_XOR) instruction = "xor"; //strcpy(instruction, "xor");
        else if (f3 == RType::FUNC3_SRL && f7 == RType::FUNC7_SRL) instruction = "srl"; //strcpy(instruction, "srl");
        else if (f3 == RType::FUNC3_SRA && f7 == RType::FUNC7_SRA) instruction = "sra"; //strcpy(instruction, "sra");
        else if (f3 == RType::FUNC3_OR && f7 == RType::FUNC7_OR) instruction = "or"; //strcpy(instruction, "or");
        else if (f3 == RType::FUNC3_AND && f7 == RType::FUNC7_AND) instruction = "and"; //strcpy(instruction, "and");
        else if (f3 == RType::FUNC3_DIV && f7 == RType::FUNC7_DIV) instruction = "div"; //strcpy(instruction, "div");
        else if (f3 == RType::FUNC3_MUL && f7 == RType::FUNC7_MUL) instruction = "mul"; //strcpy(instruction, "mul");
        else if (f3 == RType::FUNC3_MULH && f7 == RType::FUNC7_MULH) instruction = "mulh"; //strcpy(instruction, "mulh");
        else instruction = "unknown"; //strcpy(instruction, "unknown");

        wprintf(L"%S %d,%d,%d\n", instruction.c_str(), rd, rs1, rs2);
        disasm << instruction << " " << rd << "," << rs1 << "," << rs2 << std::endl;
        break;
    }

    case IType::OP_TYPE_IMMEDIATE: {
        WORD rd = (machinecode & 0xf80) >> 7;
        WORD f3 = (machinecode & 0x7000) >> 12;
        WORD rs1 = (machinecode & 0xf8000) >> 15;
        //wprintf(L"disassembled I-Type IMM instruction:\n");

        if (f3 == IType::FUNC3_SLLI || f3 == IType::FUNC3_SRLI || f3 == IType::FUNC3_SRAI) {
            WORD f6 = (machinecode & 0xfc000000) >> 26;
            WORD shamt = (machinecode & 0x3f00000) >> 20;

            if (f3 == IType::FUNC3_SLLI && f6 == IType::FUNC6_SLLI) instruction = "slli";
            else if (f3 == IType::FUNC3_SRLI && f6 == IType::FUNC6_SRLI) instruction = "srli";
            else if (f3 == IType::FUNC3_SRAI && f6 == IType::FUNC6_SRAI) instruction = "srai";

            wprintf(L"%S %d,%d,%d\n", instruction.c_str(), rd, rs1, shamt);
            disasm << instruction << " " << rd << "," << rs1 << "," << shamt << std::endl;
        }
        else {
            WORD imm12 = (machinecode & 0xfff00000) >> 20;

            if (f3 == IType::FUNC3_ADDI) instruction = "addi"; //strcpy(instruction, "addi");
            else if (f3 == IType::FUNC3_SLTI) instruction = "slti"; //strcpy(instruction, "slti");
            else if (f3 == IType::FUNC3_SLTIU) instruction = "sltiu"; //strcpy(instruction, "sltiu");
            else if (f3 == IType::FUNC3_XORI) instruction = "xori"; //strcpy(instruction, "xori");
            else if (f3 == IType::FUNC3_ORI) instruction = "ori"; //strcpy(instruction, "ori");
            else if (f3 == IType::FUNC3_ANDI) instruction = "andi"; //strcpy(instruction, "andi");
            else instruction = "unknown"; //strcpy(instruction, "unknown");

            wprintf(L"%S %d,%d\n", instruction.c_str(), rd, rs1);
            disasm << instruction << " " << rd << "," << rs1 << std::endl;
        }
        break;
    }

    case IType::OP_TYPE_LOAD: {
        WORD rd = (machinecode & 0xf80) >> 7;
        WORD f3 = (machinecode & 0x7000) >> 12;
        WORD rs1 = (machinecode & 0xf8000) >> 15;
        WORD imm12 = (machinecode & 0xfff00000) >> 20;
        //wprintf(L"disassembled I-Type LOAD instruction:\n");

        if (f3 == IType::FUNC3_LB) instruction = "lb"; //strcpy(instruction, "lb");
        else if (f3 == IType::FUNC3_LH) instruction = "lh"; //strcpy(instruction, "lh");
        else if (f3 == IType::FUNC3_LW) instruction = "lw"; //strcpy(instruction, "lw");
        else if (f3 == IType::FUNC3_LBU) instruction = "lbu"; //strcpy(instruction, "lbu");
        else if (f3 == IType::FUNC3_LHU) instruction = "lhu"; //strcpy(instruction, "lhu");
        else instruction = "unknown"; //strcpy(instruction, "unknown");

        wprintf(L"%S %d,%d\n", instruction.c_str(), rd, rs1);
        disasm << instruction << " " << rd << "," << rs1 << std::endl;
        break;
    }

    case IType::OP_JALR: {
        WORD rd = (machinecode & 0xf80) >> 7;
        WORD f3 = (machinecode & 0x7000) >> 12;
        WORD rs1 = (machinecode & 0xf8000) >> 15;
        WORD imm12 = (machinecode & 0xfff00000) >> 20;
        //wprintf(L"disassembled I-Type JALR instruction:\n");

        if (f3 == IType::FUNC3_JALR) instruction = "jalr"; //strcpy(instruction, "jalr");
        else instruction = "unknown"; //strcpy(instruction, "unknown");

        wprintf(L"%S %d,%d\n", instruction.c_str(), rd, rs1);
        disasm << instruction << " " << rd << "," << rs1 << std::endl;
        break;
    }

    case SType::OP_TYPE_STORE: {

        WORD f3 = (machinecode & 0x7000) >> 12;
        WORD rs1 = (machinecode & 0xf8000) >> 15;
        WORD rs2 = (machinecode & 0x1f00000) >> 20;

        WORD imm5 = (machinecode & 0xf80) >> 7;
        WORD imm7 = (machinecode & 0xfe000000) >> 25;
        WORD imm = (imm7 << 5) | imm5;

        //wprintf(L"disassembled S-Type instruction:\n");

        if (f3 == SType::FUNC3_SB) instruction = "sb"; //strcpy(instruction, "sb");
        else if (f3 == SType::FUNC3_SH) instruction = "sh"; //strcpy(instruction, "sh");
        else if (f3 == SType::FUNC3_SW) instruction = "sw"; //strcpy(instruction, "sw");
        else instruction = "unknown"; //strcpy(instruction, "unknown");

        wprintf(L"%S %d,%d\n", instruction.c_str(), rs2, rs1);
        disasm << instruction << " " << rs2 << "," << rs1 << std::endl;
        break;
    }

    case BType::OP_TYPE_BRANCH: {

        // first get bits for registers and f3
        WORD f3 = (machinecode & 0x7000) >> 12;
        WORD rs1 = (machinecode & 0xf8000) >> 15;
        WORD rs2 = (machinecode & 0x1f00000) >> 20;

        // then recreate the immediate correctly by moving the bits to correct position
        WORD imm12 = (machinecode & (1 << 31)) >> 20;       // move from 31 to 12
        WORD imm11 = (machinecode & (1 << 7)) << 4;         // move from 7 to 11
        WORD imm105 = (machinecode & 0x7E000000) >> 20;     // move from 30:25 to 10:5
        WORD imm41 = (machinecode & 0xF00) >> 5;             // move from 11:8 to 4:1
        WORD imm = imm12 | imm11 | imm105 | imm41;          // assemble all bits to the immediate
        //wprintf(L"disassembled B-Type instruction:\n");

        // check which instruction was done
        if (f3 == BType::FUNC3_BEQ) instruction = "beq"; //strcpy(instruction, "beq");
        else if (f3 == BType::FUNC3_BNEQ) instruction = "bneq"; //strcpy(instruction, "bneq");
        else if (f3 == BType::FUNC3_BLT) instruction = "blt"; //strcpy(instruction, "blt");
        else if (f3 == BType::FUNC3_BGE) instruction = "bge"; //strcpy(instruction, "bge");
        else if (f3 == BType::FUNC3_BLTU) instruction = "bltu"; //strcpy(instruction, "bltu");
        else if (f3 == BType::FUNC3_BGEU) instruction = "bgeu"; //strcpy(instruction, "bgeu");
        else instruction = "unknown"; //strcpy(instruction, "unknown");

        wprintf(L"%S %d,%d,%d\n", instruction.c_str(), rs1, rs2, imm);
        disasm << instruction << " " << rs1 << "," << rs2 << "," << imm << std::endl;
        break;
    }

    case UType::OP_LUI: {
        WORD rd = (machinecode & 0xf80) >> 7;
        WORD imm20 = (machinecode & 0xfffff000) >> 12;
        //wprintf(L"disassemble U-Type LUI instruction:\n31 | %d | %d | %d | 0\n", imm20, rd, opcode);
        instruction = "lui";
        wprintf(L"%S %d,%d\n", instruction.c_str(), rd, imm20);
        disasm << instruction << " " << rd << "," << imm20 << std::endl;
        break;
    }

    case JType::OP_JAL: {
        WORD rd = (machinecode & 0xf80) >> 7;

        WORD imm20 = (machinecode & (1 << 31)) >> 31;
        WORD imm1912 = (machinecode & 0xFF000) >> 12;
        WORD imm11 = (machinecode & (1 << 20)) >> 20;
        WORD imm101 = (machinecode & 0x7FE00000) >> 21;
        WORD imm = (imm20 << 19) | (imm1912 << 12) | (imm11 << 11) | (imm101 << 1);

        //wprintf(L"disassemble J-Type JAL instruction:\n31 | %d | %d | %d | 0\n", imm20, rd, opcode);
        instruction = "jal";
        wprintf(L"%S %d,%d\n", instruction.c_str(), rd, imm);
        disasm << instruction << " " << rd << "," << imm << std::endl;
        break;
    }
    case PType::OP_TYPE_PRINT: {
        WORD f3 = (machinecode & 0x7000) >> 12;
        WORD rs1 = (machinecode & 0xf8000) >> 15;

        if (f3 == PType::FUNC3_INT) instruction = "pint"; //strcpy(instruction, "pint");
        else if (f3 == PType::FUNC3_STRING) instruction = "pstr"; //strcpy(instruction, "pstr");

        wprintf(L"%S %d\n", instruction.c_str(), rs1);
        disasm << instruction << " " << rs1 << std::endl;
        break;
    }
    default:
        wprintf(L"unknown opcode");
        break;
    }
}

RiscV::BYTE RiscV::MaskOpcode(INSTRUCTION instruction)
{
    BYTE opcode = (instruction & 0x7F);
    return opcode;
}


RiscV::BYTE RiscV::MaskFunct3(INSTRUCTION instruction)
{
    BYTE f3 = (instruction & 0x7000) >> 12;
    return f3;
}

RiscV::BYTE RiscV::MaskFunct7(INSTRUCTION instruction)
{
    BYTE f7 = (instruction & 0xfe000000) >> 25;
    return f7;
}

RiscV::BYTE RiscV::MaskRs1(INSTRUCTION instruction)
{
    BYTE rs1 = (instruction & 0xf8000) >> 15;
    return rs1;
}

RiscV::BYTE RiscV::MaskRs2(INSTRUCTION instruction)
{
    BYTE rs2 = (instruction & 0x1f00000) >> 20;
    return rs2;
}

RiscV::BYTE RiscV::MaskRd(INSTRUCTION instruction)
{
    BYTE rd = (instruction & 0xf80) >> 7;
    return rd;
}


