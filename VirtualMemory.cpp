#include "VirtualMemory.h"

VirtualMemory::VirtualMemory(size_t const size)
{
	mMemory = new RiscV::WORD[RiscV::cMemDataSize];
}

VirtualMemory::~VirtualMemory() {
	delete[] mMemory;
}

RiscV::WORD VirtualMemory::Read(RiscV::ADDRESS const& address) {
	return mMemory[address];
}


void VirtualMemory::Write(RiscV::ADDRESS const& address, RiscV::WORD const& data) {
	mMemory[address] = data;
}