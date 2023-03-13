#pragma once
#include "RiscV.h"
#include "AddressRange.h"
#include <fstream>
#include <map>
#include <string>

class IVirtualDevice;

class VirtualMachine
{
public:
	VirtualMachine(std::string const& fileName, size_t regCount, bool verbose);
	~VirtualMachine();
	bool is_ready() const;
	bool RegisterDevice(IVirtualDevice* device, RiscV::ADDRESS begin, RiscV::ADDRESS end);
	void Run();

private:
	bool mVerbose = false;

	typedef std::map<AddressRange, IVirtualDevice*> TVirtualDeviceMap;
	typedef std::pair<TVirtualDeviceMap::iterator, bool> TVirtualDeviceInsertResult;
	TVirtualDeviceMap::iterator GetVirtualDevice(RiscV::ADDRESS address);

	RiscV::INSTRUCTION* mInstructionMemory;
	size_t mInstructionSize = 0;
	TVirtualDeviceMap mVirtualDeviceMap;

	RiscV::WORD ReadMemory(RiscV::ADDRESS address);
	void WriteMemory(RiscV::ADDRESS address, RiscV::WORD const& data);

	size_t const mRegCount;
	RiscV::WORD mRegisterFile[RiscV::cRegCount] = {};
	bool mRegisterFileWritten[RiscV::cRegCount] = {};
	RiscV::WORD ReadRegisterFile(size_t idx);
	void WriteRegisterFile(size_t idx, RiscV::WORD const& data);

	RiscV::ADDRESS mPc;
	bool SetPc(RiscV::ADDRESS pc);

	void PrintWarning(std::string const& message);
};

