#pragma once

#include <cstddef>
#include <tuple>

#include "Nitro.h"
#include "NCLR.h"

/*struct Pixel {
	std::pair<int, int> colorIdx;
	std::uint8_t alpha; //Needed?
};*/


struct Tile {
	std::uint16_t width = 8;
	std::uint16_t height = 8;
	bool defaultTile = true;
	std::vector<Color*> pixels;
};

class CHARBlock : public Block {
private:
	std::uint16_t height; //0x8
	std::uint16_t width; //0xa
	std::uint32_t colorFormat; //0xc

	std::uint32_t dispcnt; //0x10
	std::uint32_t tileOrder; //0x14
	std::uint32_t dataSize; //0x18
	std::uint32_t dataOffset; //0x1c

	std::unique_ptr<std::byte[]> data;

	std::vector<std::uint8_t> pixelPaletteIdx;

public:
	CHARBlock(std::ifstream& iFILE);
	CHARBlock(const CHARBlock& other);
	std::pair<std::uint16_t, std::uint16_t> getWidthAndHeight() { return { width, height }; }
	virtual void writeData(std::ofstream& oFILE) final;

	std::byte* getImageData() { return data.get(); }
	void replaceImageData(std::unique_ptr<std::byte[]>& newData);
	//void setHeight(std::uint16_t h) { height = h; }
	//void setWidth(std::uint16_t w) { width = w; }

	void setPixelPaletteIdx(std::vector<std::uint8_t>& p, std::uint16_t w, std::uint16_t h); //{ tileOrder = 1; pixelPaletteIdx = std::move(p); };
};

class CPOSBlock : public Block {
private:

public:

	virtual std::unique_ptr<Block> clone() {
		return std::make_unique<Block>(*this);
	}
	virtual void read(std::ifstream& iFILE) final {};
	virtual void writeData(std::ofstream& oFILE) final {};
};

class NCGR : public Nitro{
private:
	//std::vector<std::vector<Pixel>> pixels;
	CHARBlock* charBlock;
	int getIndex(int x, int y, int width, int height);
	std::uint8_t getClosestColor(const Color& col, std::vector<Color>& palette);
	std::vector<Tile> tiles;
public:
	NCGR(const NCGR& base);
	NCGR(std::string filename);
	
	void relinearizeData(std::vector<Color>& pixels);
	CHARBlock* getCharBlock() { return charBlock; }

	//void setImgData(std::vector<Color>& allColors, std::vector<Color>& pal, std::uint16_t width, std::uint16_t h);
	void setImgData2(std::vector<Tile>& tiles, std::vector<Color>& pal, std::uint16_t w, std::uint16_t h);

	std::tuple<std::vector<Tile>&, std::vector<std::uint16_t>> getTiles(std::vector<Color>& allColors);
	
};