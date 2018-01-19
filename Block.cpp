#include "Block.h"

Block::Block() {

}
Block::Block(const Block& other) {
	magicID = other.magicID;
	size = other.size;
}
Block& Block::operator=(const Block& other) {

	if (this == &other) {
		return *this;
	}
	magicID = other.magicID;
	size = other.size;
	return *this;
}
//In this function, we care to only read the first eight bytes/
//the magicID (4 bytes) and header size
Block::Block(std::ifstream& iFILE) {
	iFILE.read((char*)(&magicID), sizeof(magicID));
	iFILE.read((char*)(&size), sizeof(size));
}

void Block::write(std::ofstream& oFILE) {
	oFILE.write((char*)(&magicID), sizeof(magicID));
	oFILE.write((char*)(&size), sizeof(size));

	writeData(oFILE);
}