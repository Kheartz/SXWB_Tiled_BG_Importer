#pragma once

#include <vector>

#include "Block.h"

class Nitro {
private:

protected:
	std::ifstream iFILE;
	std::ofstream oFILE;
	std::uint32_t magicID;
	std::uint16_t bitOrder = 0xFFFE;
	std::uint16_t version = 0x0101;
	std::uint32_t fileSize = 0x00000000;
	std::uint16_t headerSize = 0x10;
	std::uint16_t numSections;

	std::vector<std::shared_ptr<Block>> blocks;
	virtual void writeGenericHeader();
	//virtual void writeData() = 0;
public:
	Nitro() = default;
	Nitro(const Nitro& base);
	Nitro(std::string filename);
	virtual ~Nitro() = default;

	void write(std::string filename);
};