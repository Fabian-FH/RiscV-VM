#include <cassert>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "VirtualMachine.h"
#include "IVirtualDevice.h"

VirtualMachine::VirtualMachine(std::string const& fileName, size_t regCount, bool verbose) : 
	mInstructionMemory(nullptr), mRegCount(regCount), mPc(0), mVerbose(verbose) 
{
	std::ifstream ifs(fileName, std::ios::binary | std::ios::ate);
	if (!ifs.is_open()) {
		std::cerr << "Could not open file: " << fileName << std::endl;
		return;
	}
	std::streamoff fileByteCount = ifs.tellg();
	/*if ((fileByteCount & 1) != 0) {
		std::cerr << "Invalid binary" << std::endl;
		return;
	}*/
	mInstructionSize = static_cast<size_t>(fileByteCount / RiscV::cDataIncrement);
	ifs.seekg(0, std::ios::beg);
	mInstructionMemory = new RiscV::INSTRUCTION[mInstructionSize];
	ifs.read(reinterpret_cast<char*>(mInstructionMemory), fileByteCount);
	ifs.close();

	for (size_t i = 0; i < RiscV::cRegCount; ++i) {
		mRegisterFileWritten[i] = false;
	}
}

VirtualMachine::~VirtualMachine() {
	delete[] mInstructionMemory;
}

bool VirtualMachine::is_ready() const {
	return mInstructionMemory != nullptr;
}

bool VirtualMachine::RegisterDevice(IVirtualDevice* device, RiscV::ADDRESS begin, RiscV::ADDRESS end) {
	TVirtualDeviceInsertResult result = mVirtualDeviceMap.insert(TVirtualDeviceMap::value_type(AddressRange(begin, end), device));
	assert(result.second);
	return result.second;
}

void VirtualMachine::PrintWarning(std::string const& message) {
	std::cerr << "warning at pc 0x" << std::setfill('0') << std::setw(4) << std::hex << mPc << ": "
		<< message << std::endl;
}

RiscV::WORD VirtualMachine::ReadRegisterFile(size_t idx) {
	if (idx >= mRegCount) {
		std::ostringstream oss;
		oss << "using higher register index than allowed index " << (mRegCount - 1);
		PrintWarning(oss.str());
	}
	if (!mRegisterFileWritten[idx]) {
		std::ostringstream oss;
		oss << "register index " << idx << " has not been used yet and has undefined value";
		PrintWarning(oss.str());
	}
	return mRegisterFile[idx];
}

void VirtualMachine::WriteRegisterFile(size_t idx, RiscV::WORD const& data) {
	if (idx >= mRegCount) {
		std::ostringstream oss;
		oss << "using higher register index than allowed index " << (mRegCount - 1);
		PrintWarning(oss.str());
	}
	mRegisterFile[idx] = data;
	mRegisterFileWritten[idx] = true;
}

VirtualMachine::TVirtualDeviceMap::iterator VirtualMachine::GetVirtualDevice(RiscV::ADDRESS address) {
	TVirtualDeviceMap::iterator iter = mVirtualDeviceMap.lower_bound(AddressRange(address, address));
	if (iter == mVirtualDeviceMap.end() || address < iter->first.Begin() || address > iter->first.End()) {
		return mVirtualDeviceMap.end();
	}
	return iter;
}

RiscV::WORD VirtualMachine::ReadMemory(RiscV::ADDRESS address) {
	TVirtualDeviceMap::iterator iter = GetVirtualDevice(address);
	if (iter == mVirtualDeviceMap.end()) {
		std::ostringstream oss;
		oss << "read from undefined memory address 0x" << std::setfill('0') << std::setw(4) << std::hex << address;
		PrintWarning(oss.str());
		return 0;
	}
	return iter->second->Read(address - iter->first.Begin());
}

void VirtualMachine::WriteMemory(RiscV::ADDRESS address, RiscV::WORD const& data) {
	TVirtualDeviceMap::iterator iter = GetVirtualDevice(address);
	if (iter == mVirtualDeviceMap.end()) {
		std::ostringstream oss;
		oss << "write to undefined memory address 0x" << std::setfill('0') << std::setw(4) << std::hex << address;
		PrintWarning(oss.str());
		return;
	}
	iter->second->Write(address - iter->first.Begin(), data);
}

bool VirtualMachine::SetPc(RiscV::ADDRESS pc) {
	bool pcOutOfRange = pc >= mInstructionSize;
	if (pcOutOfRange) {
		std::string warning = "program counter went out of range (0d" + std::to_string(pc) + "), stopping virtual machine";
		PrintWarning(warning);
	}
	else {
		mPc = pc;
	}
	return !pcOutOfRange;
}

using namespace RiscV;

void VirtualMachine::Run() {

	// run until either PC oversteps all instructions or 
	// a sleep statement was reached
	while (mPc < mInstructionSize) {

		RiscV::INSTRUCTION inst = mInstructionMemory[mPc];
		if(mVerbose) std::cout << std::endl << "0x" << std::setfill('0') << std::setw(4) << std::hex << mPc << ": ";
		

		// first get all parts of the instruction
		// some of these variables might be invalid for the current instruction,
		// but like this the logic inside the switch-case can be reduced

		RiscV::BYTE opcode = RiscV::MaskOpcode(inst);
		RiscV::BYTE f3 = RiscV::MaskFunct3(inst);
		RiscV::BYTE f7 = RiscV::MaskFunct7(inst);
		RiscV::BYTE rd = RiscV::MaskRd(inst);
		RiscV::BYTE rs1 = RiscV::MaskRs1(inst);
		RiscV::BYTE rs2 = RiscV::MaskRs2(inst);

		// I-Type immediates

		RiscV::WORD imm_11to0 = (inst & 0xfff00000) >> 20;
		RiscV::WORD f6 = (inst & 0xfc000000) >> 26;
		RiscV::WORD shamt = (inst & 0x3f00000) >> 20;

		// S-Type immediates

		RiscV::BYTE imm_4to0 = (inst & 0xf80) >> 7;
		RiscV::BYTE imm_11to5 = (inst & 0xfe000000) >> 25;

		// B-Type immediates

		RiscV::BYTE imm11b = (inst & (1 << 31)) >> 31;       
		RiscV::BYTE imm10b = (inst & (1 << 7)) >> 7;         
		RiscV::BYTE imm_9to4 = (inst & 0x7E000000) >> 25;     
		RiscV::BYTE imm_3to0 = (inst & 0xF00) >> 8;     


		// U-Type immediates

		RiscV::WORD imm_31to12 = (inst & 0xfffff000) >> 12;

		// J-Type immediates
		
		RiscV::BYTE imm19 = (inst & (1 << 31)) >> 31;
		RiscV::BYTE imm_1811 = (inst & 0xFF000) >> 12;
		RiscV::BYTE imm10j = (inst & (1 << 20)) >> 20;
		RiscV::WORD imm_9to0 = (inst & 0x7FE00000) >> 21;

		bool executeJump = false;

		switch (opcode) {
		case RiscV::PType::OP_TYPE_SLEEP: {
			std::cerr << "info at pc 0x" << std::setfill('0') << std::setw(4) << std::hex << mPc << ": sleep instruction reached, ending execution" << std::endl;
			return;
			break;
		}
		case RiscV::PType::OP_TYPE_PRINT: {
			// print according to f3
			if (f3 == RiscV::PType::FUNC3_INT) {
				std::cout << (int)ReadRegisterFile(rs1) << std::endl;
			}
			else if (f3 == RiscV::PType::FUNC3_STRING) {
				// for the date of this implementation, string == int
				std::cout << ReadRegisterFile(rs1) << std::endl;
			}
			break;
		}
		case RiscV::RType::OP_TYPE_REGISTER: {
			if (f3 == RiscV::RType::FUNC3_ADD && f7 == RiscV::RType::FUNC7_ADD) {
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::WORD res = valRs1 + valRs2;
				WriteRegisterFile(rd, res);

				if (mVerbose) std::cout << "add" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2 << "     ; res=" << res;
			}
			else if (f3 == RiscV::RType::FUNC3_SUB && f7 == RiscV::RType::FUNC7_SUB) {
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::WORD res = valRs1 - valRs2;
				WriteRegisterFile(rd, res);

				if (mVerbose) std::cout << "sub" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RiscV::RType::FUNC3_SLL && f7 == RiscV::RType::FUNC7_SLL) {
				// shift left logical: shift x[rs1] left by x[rs2] bit positions, result into rd
				// only bits 4:0 of x[rs2] are the shift amount, upper bits are ignored
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::BYTE shamt = (valRs2 & 0x1F); // take the lower 5 bits of value as shamt
				RiscV::WORD result = valRs1 << shamt;
				WriteRegisterFile(rd, result);

				if (mVerbose) std::cout << "sll" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_SLT && f7 == RType::FUNC7_SLT) {
				// compare rs1 and rs2 as signed numbers
				// write 1 to rd ir rs1 < rs2, write 0 to rd if rs1 > rs2
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::WORD res = (valRs1 < valRs2) ? 1 : 0;
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "slt" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_SLTU && f7 == RType::FUNC7_SLTU) {
				// compare rs1 and rs2 as unsigned numbers
				// write 1 to rd ir rs1 < rs2, write 0 to rd if rs1 > rs2
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::WORD res = ((uint32_t)valRs1 < (uint32_t)valRs2) ? 1 : 0;
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "sltu" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_XOR && f7 == RType::FUNC7_XOR) {
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::WORD res = valRs1 ^ valRs2;
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "xor" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_SRL && f7 == RType::FUNC7_SRL) {
				// shift logical right: shift x[rs1] right by x[rs2] bit positions, result into rd
				// only bits 4:0 of x[rs2] are the shift amount, upper bits are ignored
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::BYTE shamt = (valRs2 & 0x1F);
				RiscV::WORD result = (unsigned)valRs1 >> shamt;
				WriteRegisterFile(rd, result);

				if (mVerbose) std::cout << "srl" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_SRA && f7 == RType::FUNC7_SRA) {
				// shift arithmetic right: shift x[rs1] right by x[rs2] bit positions, result into rd
				// only bits 4:0 of x[rs2] are the shift amount, upper bits are ignored
				// the vacated bits are filled with copies of x[rs1] most-significant bit
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::BYTE shamt = (valRs2 & 0x1F);
				RiscV::WORD result = 0;

				if (valRs1 < 0 && shamt > 0) {
					result = valRs1 >> shamt | ~(~0U >> shamt);
				}
				else {
					result = valRs1 >> shamt;
				}
				
				WriteRegisterFile(rd, result);
				if (mVerbose) std::cout << "sra" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_OR && f7 == RType::FUNC7_OR) {
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::WORD res = valRs1 | valRs2;
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "or" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_AND && f7 == RType::FUNC7_AND) {
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::WORD res = valRs1 & valRs2;
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "and" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_DIV && f7 == RType::FUNC7_DIV) {
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				if (valRs2 == 0) {
					PrintWarning("trying to divide through 0, not executing instruction");
					// set result code to "error/failed"
					break;
				}
				RiscV::WORD res = valRs1 / valRs2;
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "div" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_DIVU && f7 == RType::FUNC7_DIVU) {
				uint32_t valRs1 = ReadRegisterFile(rs1);
				uint32_t valRs2 = ReadRegisterFile(rs2);
				if (valRs2 == 0) {
					PrintWarning("trying to divide through 0, setting value to 1 instead");
					valRs2 = 1;
				}
				RiscV::WORD res = valRs1 / valRs2;
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "divu" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_REM && f7 == RType::FUNC7_REM) {
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				if (valRs2 == 0) {
					PrintWarning("trying to modulo through 0, setting value to 1 instead");
					valRs2 = 1;
				}
				RiscV::WORD res = valRs1 % valRs2;
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "rem" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_REMU && f7 == RType::FUNC7_REMU) {
				uint32_t valRs1 = ReadRegisterFile(rs1);
				uint32_t valRs2 = ReadRegisterFile(rs2);
				if (valRs2 == 0) {
					PrintWarning("trying to modulo through 0, setting value to 1 instead");
					valRs2 = 1;
				}
				RiscV::WORD res = valRs1 % valRs2;
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "remu" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_MUL && f7 == RType::FUNC7_MUL) {
				// multiplication creates a WORD + WORD = 64 Bit value
				// mul returns the lower 32 Bit of the 64 Bit result
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::WORD res = ((int64_t)valRs1 * (int64_t)valRs2) & 0xFFFFFFFF;
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "mul" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_MULH && f7 == RType::FUNC7_MULH) {
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				int64_t mult = (int64_t)valRs1 * (int64_t)valRs2;
				RiscV::WORD res = (mult & 0xFFFFFFFF00000000) >> 32; // take upper 32 bits of 64 bit result 
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "mulh" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_MULHSU && f7 == RType::FUNC7_MULHSU) {
				//rs1 as signed and rs2 as unsigned number, else like mulh
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::WORD res = (((int64_t)valRs1 * (uint64_t)valRs2) & 0xFFFFFFFF00000000) >> 32; // take upper 32 bits of 64 bit result 
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "mulhsu" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			else if (f3 == RType::FUNC3_MULHU && f7 == RType::FUNC7_MULHU) {
				// rs1 and rs2 as unsigned number, else like mulh
				RiscV::WORD valRs1 = ReadRegisterFile(rs1);
				RiscV::WORD valRs2 = ReadRegisterFile(rs2);
				RiscV::WORD res = (((uint64_t)valRs1 * (uint64_t)valRs2) & 0xFFFFFFFF00000000) >> 32; // take upper 32 bits of 64 bit result 
				WriteRegisterFile(rd, res);
				if (mVerbose) std::cout << "mulhu" << " r" << (int)rd << ",r" << (int)rs1 << ",r" << (int)rs2;
			}
			break;
		}

			case IType::OP_TYPE_IMMEDIATE: {
				if (f3 == IType::FUNC3_SLLI || f3 == IType::FUNC3_SRLI || f3 == IType::FUNC3_SRAI) {

					if (f3 == IType::FUNC3_SLLI && f6 == IType::FUNC6_SLLI) {
						RiscV::WORD valRs1 = ReadRegisterFile(rs1);
						RiscV::WORD result = 0;
						// for rv32I, instruction is only legal when shamt[5]==0 
						if (((shamt & 0x10) >> 5) == 0) { 
							result = valRs1 << shamt;
						}
						else {
							PrintWarning("illegal shift amount, rd=rs1");
							result = valRs1;
						}
						WriteRegisterFile(rd, result);

						if (mVerbose) std::cout << "slli" << " r" << (int)rd << ",r" << (int)rs1 << "," << (int)shamt;
					}
					else if (f3 == IType::FUNC3_SRLI && f6 == IType::FUNC6_SRLI) {
						// shift right logical
						RiscV::WORD valRs1 = ReadRegisterFile(rs1);
						RiscV::WORD valRs2 = ReadRegisterFile(rs2);
						RiscV::WORD result = 0;

						// for rv32I, instruction is only legal when shamt[5]==0 
						if (((shamt & 0x10) >> 5) == 0) {
							result = (unsigned)valRs1 >> shamt;
						}
						else {
							PrintWarning("illegal shift amount, rd=rs1");
							result = valRs1;
						}
						
						WriteRegisterFile(rd, result);
						if (mVerbose) std::cout << "srli" << " r" << (int)rd << ",r" << (int)rs1 << "," << (int)shamt;
					}
					else if (f3 == IType::FUNC3_SRAI && f6 == IType::FUNC6_SRAI) {
						RiscV::WORD valRs1 = ReadRegisterFile(rs1);
						RiscV::WORD result = 0;
						// for rv32I, instruction is only legal when shamt[5]==0 
						if (((shamt & 0x10) >> 5) == 0) {

							// see SRA
							if (valRs1 < 0 && shamt > 0) {
								result = valRs1 >> shamt | ~(~0U >> shamt);
							}
							else {
								result = valRs1 >> shamt;
							}
						}
						else {
							PrintWarning("illegal shift amount, rd=rs1");
							result = valRs1;
						}
						WriteRegisterFile(rd, result);
						if (mVerbose) std::cout << "srai" << " r" << (int)rd << ",r" << (int)rs1 << "," << (int)shamt;
					}
				}
				else {
					if (f3 == IType::FUNC3_ADDI) {
						RiscV::WORD valRs1 = ReadRegisterFile(rs1);
						RiscV::WORD res = valRs1 + imm_11to0;
						WriteRegisterFile(rd, res);
						if (mVerbose) std::cout << "addi" << " r" << (int)rd << ",r" << (int)rs1 << "," << imm_11to0;
					}
					else if (f3 == IType::FUNC3_SLTI) {
						RiscV::WORD valRs1 = ReadRegisterFile(rs1);
						RiscV::WORD res = (valRs1 < imm_11to0) ? 1 : 0;
						WriteRegisterFile(rd, res);
						if (mVerbose) std::cout << "slti" << " r" << (int)rd << ",r" << (int)rs1 << "," << (int)imm_11to0;
					}
					else if (f3 == IType::FUNC3_SLTIU) {
						RiscV::WORD valRs1 = ReadRegisterFile(rs1);
						RiscV::WORD res = ((uint32_t)valRs1 < (uint32_t)imm_11to0) ? 1 : 0;
						WriteRegisterFile(rd, res);
						if (mVerbose) std::cout << "sltiu" << " r" << (int)rd << ",r" << (int)rs1 << "," << (int)imm_11to0;
					}
					else if (f3 == IType::FUNC3_XORI) {
						RiscV::WORD valRs1 = ReadRegisterFile(rs1);
						RiscV::WORD res = valRs1 ^ imm_11to0;
						WriteRegisterFile(rd, res);
						if (mVerbose) std::cout << "xori" << " r" << (int)rd << ",r" << (int)rs1 << "," << imm_11to0;
					}
					else if (f3 == IType::FUNC3_ORI) {
						RiscV::WORD valRs1 = ReadRegisterFile(rs1);
						RiscV::WORD res = valRs1 | imm_11to0;
						WriteRegisterFile(rd, res);
						if (mVerbose) std::cout << "ori" << " r" << (int)rd << ",r" << (int)rs1 << "," << imm_11to0;
					}
					else if (f3 == IType::FUNC3_ANDI) {
						RiscV::WORD valRs1 = ReadRegisterFile(rs1);
						RiscV::WORD res = valRs1 & imm_11to0;
						WriteRegisterFile(rd, res);
						if (mVerbose) std::cout << "andi" << " r" << (int)rd << ",r" << (int)rs1 << "," << imm_11to0;
					}
				}
				break;
			}

			case IType::OP_TYPE_LOAD: {
				// for the date of implementation, only lw is needed
				// for the future, the IVirtualDevice's "Read()" needs to be given a parameter 
				// for how many bytes should be read from memory or exist in multiple versions
				/*if (f3 == IType::FUNC3_LB) {

				}
				else if (f3 == IType::FUNC3_LH) {

				}
				else if (f3 == IType::FUNC3_LW) {

				}
				else if (f3 == IType::FUNC3_LBU) {

				}
				else if (f3 == IType::FUNC3_LHU) {

				}*/

				WORD addr = ReadRegisterFile(rs1) + imm_11to0;	// get target address from rs1
				WORD data = ReadMemory(addr);		// read the data from memory
				WriteRegisterFile(rd, data);
				if (mVerbose) std::cout << "lw" << " r" << (int)rd << ",[r" << (int)rs1 << "]+" << (int)imm_11to0 << "     ; data=" << data << ", addr=" << addr;
				break;
			}

			case IType::OP_JALR: {
				if (f3 == IType::FUNC3_JALR) {
					// JALR: jumps to the address in rs1 + offset

					executeJump = true;
					//RiscV::WORD alignedOffset = imm_11to0 & ~0x1; // set the least bit to 0
					//RiscV::BYTE addr = ReadRegisterFile(rs1) + alignedOffset;
					RiscV::BYTE addr = ReadRegisterFile(rs1) + imm_11to0;

					WriteRegisterFile(rd, mPc); // save the current PC into rd before jump
					if (!SetPc(addr)) return;

					if (mVerbose) std::cout << "jalr" << " r" << (int)rd << ",#" << (int)addr << "     ; new PC=" << mPc;
				}
				break;
			}

			case SType::OP_TYPE_STORE: {
				// first recreate immediate!!!
				WORD imm = (imm_11to5 << 5) | imm_4to0; // offset

				// for the date of implementation, only sw is needed
				// for the future, the IVirtualDevice's "Write()" needs to be given a parameter 
				// for how many bytes should be written to memory or exist in multiple versions
				/*if (f3 == SType::FUNC3_SB) {

				}
				else if (f3 == SType::FUNC3_SH) {

				}
				else if (f3 == SType::FUNC3_SW) {

				}*/

				// in RISCV, rs2 holds the data and rs1 holds the address in memory
				// store the four ls bytes of rs2 to memory at addres rs1 + offset
				WORD addr = ReadRegisterFile(rs1) + imm;
				WORD data = ReadRegisterFile(rs2);
				WriteMemory(addr, data);
				if (mVerbose) std::cout << "sw" << " r" << (int)rs2 << ",[r" << (int)rs1 << "]+" << imm << "     ; data=" << data << ", " << "addr=" << addr;

				break;
			}

			case BType::OP_TYPE_BRANCH: {

				// first recreate immediate!!!
				WORD offset = ((imm11b << 11) | (imm10b << 10) | (imm_9to4 << 4) | imm_3to0 );

				WORD valRs1 = ReadRegisterFile(rs1);
				WORD valRs2 = ReadRegisterFile(rs2);
				executeJump = true;
				
				// check which instruction was done
				if (f3 == BType::FUNC3_BEQ) {
					if (valRs1 == valRs2) {
						if (!SetPc(offset)) return;
					}
					else {
						// since the condition was not true, do not jump
						executeJump = false;
					}
					if (mVerbose) std::cout << "beq" << " r" << (int)rs1 << ",r" << (int)rs2 << ",#" << offset <<  "    ; new PC=" << mPc;
				}
				else if (f3 == BType::FUNC3_BNEQ) {
					if (valRs1 != valRs2) {
						if (!SetPc(offset)) return;
					}
					else {
						// since the condition was not true, do not jump and move to next pc
						executeJump = false;
					}
					if (mVerbose) std::cout << "bneq" << " r" << (int)rs1 << ",r" << (int)rs2 << ",#" << offset << "   ; new PC=" << mPc;
				}
				else if (f3 == BType::FUNC3_BLT) {
					if (valRs1 < valRs2) {
						if (!SetPc(offset)) return;
					}
					else {
						executeJump = false;
					}
					if (mVerbose) std::cout << "blt" << " r" << (int)rs1 << ",r" << (int)rs2 << ",#" << offset << "    ; new PC=" << mPc;
				}
				else if (f3 == BType::FUNC3_BGE) {
					if (valRs1 >= valRs2) {
						if (!SetPc(offset)) return;
					}
					else {
						executeJump = false;
					}
					if (mVerbose) std::cout << "bge" << " r" << (int)rs1 << ",r" << (int)rs2 << ",#" << offset << "    ; new PC=" << mPc;
				}
				else if (f3 == BType::FUNC3_BLTU) {
					if ((uint32_t)valRs1 == (uint32_t)valRs2) {
						if (!SetPc(offset)) return;
					}
					else {
						executeJump = false;
					}
					if (mVerbose) std::cout << "bltu" << " r" << (int)rs1 << ",r" << (int)rs2 << ",#" << offset << "   ; new PC=" << mPc;
				}
				else if (f3 == BType::FUNC3_BGEU) {
					if ((uint32_t)valRs1 >= (uint32_t)valRs2) {
						if (!SetPc(offset)) return;
					}
					else {
						executeJump = false;
					}
					if (mVerbose) std::cout << "bgeu" << " r" << (int)rs1 << ",r" << (int)rs2 << ",#" << offset << "   ; new PC=" << mPc;
				}
				break;
			}

			case UType::OP_LUI: {
				RiscV::WORD upperIm = (imm_31to12 << 12);
				WriteRegisterFile(rd, upperIm);
				if (mVerbose) std::cout << "lui" << " r" << (int)rd << "," << upperIm;
				break;
			}

			case JType::OP_JAL: {
				// JAL: jump directly to given offset
				
				// first recreate immediate!!!
				// J-Type has 20-bit immediate
				WORD offset = (imm19 << 18) | (imm_1811 << 11) | (imm10j << 10) | imm_9to0;
				executeJump = true;

				WriteRegisterFile(rd, mPc + 1); // save the address of the next instruction to rd
				if (!SetPc(offset)) return;	// set program counter to target == jump to target
				if (mVerbose) std::cout << "jal" << " r" << (int)rd << ",#" << offset << "       ; new PC=" << mPc;;
				break;
			}
			default:
				PrintWarning("unknown opcode");
				break;
		}

		// if there was no valid jump instruction, move on to the next PC
		if (!executeJump) {
			if (!SetPc(mPc + 1)) return;
		}
	}
}
