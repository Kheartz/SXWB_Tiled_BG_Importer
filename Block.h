#pragma once

#include <fstream>
#include <memory>
#include <iostream>
class Block {
private:

protected:
	std::uint32_t magicID;
	std::uint32_t size;
public:
	Block();
	Block(std::ifstream& iFILE);
	Block(const Block& other);
	Block& operator=(const Block& other);
	//void readPreHeader(std::ifstream& iFILE);
	void write(std::ofstream& oFILE);
	virtual void writeData(std::ofstream& oFILE) {};
};