#pragma once
#include "RiscV.h"

class IVirtualDevice {
public:
	virtual RiscV::WORD Read(RiscV::ADDRESS const& address) = 0;
	virtual void Write(RiscV::ADDRESS const& address, RiscV::WORD const& data) = 0;
};