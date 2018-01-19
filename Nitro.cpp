#include "Nitro.h"

Nitro::Nitro(std::string filename) {
	//Read Header
	iFILE.open(filename, std::iostream::binary);
	iFILE.read((char*)(&magicID), sizeof(magicID));
	iFILE.read((char*)(&bitOrder), sizeof(bitOrder));
	iFILE.read((char*)(&version), sizeof(version));
	iFILE.read((char*)(&fileSize), sizeof(fileSize));
	iFILE.read((char*)(&headerSize), sizeof(headerSize));
	iFILE.read((char*)(&numSections), sizeof(numSections));
}
Nitro::Nitro(const Nitro& base) {
	magicID = base.magicID;
	bitOrder = base.bitOrder;
	version = base.version;
	fileSize = 0x00000000;
	headerSize = base.headerSize;
	numSections = base.numSections;

	//blocks = base.blocks;
}

void Nitro::write(std::string filename) {
	oFILE.open(filename, std::ostream::binary);
	writeGenericHeader();
	for (auto& block : blocks) {
		block->write(oFILE);
	}
	std::streampos finalPos = oFILE.tellp();
	oFILE.seekp(0x8);
	std::uint32_t finalPosInt = static_cast<std::uint32_t>(finalPos);
	oFILE.write((char*)(&finalPosInt), sizeof(std::uint32_t));
	oFILE.close();
}

void Nitro::writeGenericHeader() {

	//Write in Generic Header
	
	oFILE.write((char*)(&magicID), sizeof(magicID));
	oFILE.write((char*)(&bitOrder), sizeof(bitOrder));
	oFILE.write((char*)(&version), sizeof(version));
	oFILE.write((char*)(&fileSize), sizeof(fileSize));
	oFILE.write((char*)(&headerSize), sizeof(headerSize));
	oFILE.write((char*)(&numSections), sizeof(numSections));

}