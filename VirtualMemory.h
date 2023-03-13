#pragma once
#include "IVirtualDevice.h"
#include <vector>
class VirtualMemory : public IVirtualDevice
{
public:
	VirtualMemory(size_t const size);
	~VirtualMemory();
	virtual RiscV::WORD Read(RiscV::ADDRESS const& address);
	virtual void Write(RiscV::ADDRESS const& address, RiscV::WORD const& data);
private:
	RiscV::WORD* mMemory;
};

