#include <iostream>
#include <fstream>
#include <string>
#include "VirtualMachine.h"
#include "VirtualMemory.h"
#include "RiscV.h"


int main(int argc, char* argv[]) {

	if (argc < 2 || argc > 4) {
		std::cerr << "Usage:" << std::endl;
		std::cerr << "\t" << argv[0] << " <riscv binaryfile> [number of registers] [-v]" << std::endl;
		return 1;
	}

	// get and check input file 
	std::string fileName(argv[1]);
	std::cout << "executing file: " << fileName << std::endl;

	// get and check optional parameters
	size_t numRegisters = RiscV::cRegCount;
	bool verboseMode = false;

	for (int i = 2; i < argc; i++)
	{
		char* currArg = argv[i];
		if (strcmp(currArg, "-v") == 0) {
			verboseMode = true;
		}
		else {
			try {
				numRegisters = std::stoi(currArg);
				if (numRegisters < 1 || numRegisters > RiscV::cRegCount) {
					std::cerr << "Register count must be between 1 and 32" << std::endl;
					return 3;
				}
			}
			catch(...){
				std::cerr << "Register count must be a int number" << std::endl;
				return 3;
			}
		}
	}
	

	VirtualMachine RiscVvm(std::string(argv[1]), numRegisters, verboseMode);
	if (RiscVvm.is_ready()) {

		VirtualMemory* virtualMemory = new VirtualMemory(RiscV::cMemDataSize);
		RiscVvm.RegisterDevice(virtualMemory, 0x0000, 0x7fff);
		RiscVvm.Run();
		delete virtualMemory;
	}
	else {
		return -1;
	}
	return 0;
}