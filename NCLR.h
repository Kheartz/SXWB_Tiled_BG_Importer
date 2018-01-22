#pragma once

#include "Nitro.h"
#include <iostream>
#include <tuple>
#include <bitset>
struct Color {
	std::uint8_t blue;
	std::uint8_t green;
	std::uint8_t red;
	std::uint8_t alpha;
	Color(std::uint8_t b, std::uint8_t g, std::uint8_t r, std::uint8_t a) {
		blue = b;
		green = g;
		red = r;
		alpha = a;
	}
	bool operator==(const Color& other) {
		return blue == other.blue && green == other.green && red == other.red && alpha == other.alpha;
	}
};



enum ColorFormat {
	Indexed_4bpp = 3,	// 4  bits for 16  colors
	Indexed_8bpp = 4,	// 8  bits for 256 colors          
};

class PLTTBlock : public Block {
private:
	std::uint32_t depth;
	std::uint32_t extendedPalette;
	std::uint32_t adjustedSize;
	std::uint32_t paletteOffset;
	std::unique_ptr<uint16_t[]> paletteData; //BGR555 format transformed from the Palette data
	std::uint32_t paletteSize; //In Bytes = Colors * 2
	
	std::uint16_t getBGR555(Color& c);
	
public:
	PLTTBlock(std::ifstream& iFILE);
	PLTTBlock(const PLTTBlock& other);
	virtual void writeData(std::ofstream& oFILE) final;
	std::uint32_t getDepth() const { return depth; }
	void populatePaletteData(std::vector<std::vector<Color>>& c);
	void changePaletteData(const std::uint16_t* pData, std::uint32_t size);
	std::tuple<std::uint16_t*, std::uint32_t> getPaletteData();// { return std::forward_as_tuple(paletteData.get(), paletteSize); }
};

class PCMPBlock : public Block {
private:
	std::uint16_t numPalettes;
	std::uint16_t constant;
	std::uint32_t dataOffset;
	std::unique_ptr<std::uint16_t[]> paletteIndex;
public:
	PCMPBlock(std::ifstream& iFILE);
	PCMPBlock(const PCMPBlock& other);
	virtual void writeData(std::ofstream& oFILE) final;
	std::uint16_t* getIndex() const { return paletteIndex.get(); }
};


class NCLR : public Nitro {
private:
	PLTTBlock* palBlk;
	std::vector<std::vector<Color>> palette; //Color data straight from CIMG
	virtual void writeData() {};
public:
	NCLR(const NCLR& base, bool changePalette = true);
	NCLR(std::string filename);

	std::vector<std::vector<Color>>& getPalette() { return palette; }

	void setPalette(std::vector<Color>& pal);
	void setPalette(std::vector<std::vector<Color>>& pal);

	//Color getClosestColor(const Color& col);

	PLTTBlock* getPalBlock() { return palBlk; }

};