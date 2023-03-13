#pragma once
#include "RiscV.h"

class AddressRange
{
public:
	AddressRange(RiscV::ADDRESS begin, RiscV::ADDRESS end);
	bool operator<(AddressRange const& adressRange) const;
	RiscV::ADDRESS Begin() const;
	RiscV::ADDRESS End() const;
private:
	RiscV::ADDRESS const mBegin;
	RiscV::ADDRESS const mEnd;
};

