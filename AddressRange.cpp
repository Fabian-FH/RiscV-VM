#include "AddressRange.h"

#include <cassert>


AddressRange::AddressRange(RiscV::ADDRESS begin, RiscV::ADDRESS end) : mBegin(begin), mEnd(end)
{
	assert(begin <= end);
}

bool AddressRange::operator<(AddressRange const& adressRange) const {
	return mEnd < adressRange.mBegin;
}

RiscV::ADDRESS AddressRange::Begin() const {
	return mBegin;
}

RiscV::ADDRESS AddressRange::End() const {
	return mEnd;
}