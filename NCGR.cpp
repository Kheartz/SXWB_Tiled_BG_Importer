#include <typeinfo>
#include <iostream>
#include <bitset>
using namespace std;
#include "NCGR.h"

//First 8 bytes of the block have already been open and read.
CHARBlock::CHARBlock(std::ifstream& iFILE) : Block(iFILE){
	std::streampos blockPos = iFILE.tellg();
	iFILE.read((char*)(&height), sizeof(height)); //0x8
	iFILE.read((char*)(&width), sizeof(width)); //0xa
	iFILE.read((char*)(&colorFormat), sizeof(colorFormat)); //0xc
	
	iFILE.read((char*)(&dispcnt), sizeof(dispcnt)); //0x10
	iFILE.read((char*)(&tileOrder), sizeof(tileOrder)); //0x14
	iFILE.read((char*)(&dataSize), sizeof(dataSize)); //0x18
	iFILE.read((char*)(&dataOffset), sizeof(dataOffset)); //0x1c


	data = std::make_unique<std::byte[]>(dataSize);
	iFILE.read((char*)data.get(), dataSize);
}
CHARBlock::CHARBlock(const CHARBlock& other) : Block(other){
	height = other.height;
	width = other.width;
	colorFormat = other.colorFormat;

	dispcnt = other.dispcnt;
	tileOrder = other.tileOrder;
	dataSize = other.dataSize;
	dataOffset = other.dataOffset;

	data = std::make_unique<std::byte[]>(dataSize);
	std::memcpy(data.get(), other.data.get(), dataSize);
}

void CHARBlock::writeData(std::ofstream& oFILE) {
	oFILE.write((char*)(&height), sizeof(height)); //0x8
	oFILE.write((char*)(&width), sizeof(width)); //0xa
	oFILE.write((char*)(&colorFormat), sizeof(colorFormat)); //0xc

	oFILE.write((char*)(&dispcnt), sizeof(dispcnt)); //0x10
	oFILE.write((char*)(&tileOrder), sizeof(tileOrder)); //0x14
	std::streampos dataPos = oFILE.tellp();
	oFILE.write((char*)(&dataSize), sizeof(dataSize)); //0x18
	oFILE.write((char*)(&dataOffset), sizeof(dataOffset)); //0x1c

	//Assuming 4bpp for now
	for (unsigned int i = 0; i < pixelPaletteIdx.size(); i++) {
		std::vector<std::uint8_t>& pixelIndices = std::get<1>(pixelPaletteIdx[i]);
		for (unsigned int p = 0; p < pixelIndices.size(); p+=2) {
			std::uint8_t& palIdx1 = pixelIndices[p];
			std::uint8_t& palIdx2 = pixelIndices[p + 1];
			//std::uint8_t b = (palIdx1 << 4) | palIdx2;
			std::uint8_t b = (palIdx2 << 4) | palIdx1;
			oFILE.write((char*)(&b), sizeof(std::uint8_t));
		}
	}
	dataSize = (std::uint32_t)(pixelPaletteIdx.size() * 64 / 2);

	std::streampos currPos = oFILE.tellp();
	oFILE.seekp(dataPos);
	oFILE.write((char*)(&dataSize), sizeof(dataSize));

	std::streampos sectionSizePos = 0x14;
	std::uint32_t newSectionSize = 0x20 + dataSize;
	oFILE.seekp(sectionSizePos);
	oFILE.write((char*)(&newSectionSize), sizeof(newSectionSize));


	oFILE.seekp(currPos);

}
void CHARBlock::replaceImageData(std::unique_ptr<std::byte[]>& newData) {
	data = std::move(newData);
}

//Generic Header preread
NCGR::NCGR(std::string filename) : Nitro(filename) {
	blocks.reserve(numSections);

	blocks.emplace_back(std::make_unique<CHARBlock>(iFILE));
	if (numSections == 2) {
		blocks.emplace_back(std::make_unique<CPOSBlock>());
	}
	
}

void CHARBlock::setPixelPaletteIdx(std::vector<std::tuple<std::uint8_t, std::vector<std::uint8_t>>>& p, std::uint16_t w, std::uint16_t h){
	tileOrder = 0; 
	width = w; //fix at 256
	//height = h; //change the height until %
	while (p.size() % 32 != 0) {
		p.emplace_back(p[0]);
	}
	height = (std::uint8_t)(p.size() / width);



	pixelPaletteIdx = std::move(p); 

};

NCGR::NCGR(const NCGR& base) : Nitro(base) {

	if (base.blocks.size() > 0 && base.blocks[0] != nullptr) {
		std::unique_ptr<CHARBlock> newCHARBlock = std::make_unique<CHARBlock>(*(dynamic_cast<CHARBlock*>(base.blocks[0].get())));
		blocks.emplace_back(std::move(newCHARBlock));
		charBlock = dynamic_cast<CHARBlock*>(blocks[0].get());
	}

}

int NCGR::getIndex(int x, int y, int width, int height) {
	int index = 0;
	int tileLength = 8 * 8;
	int numTilesX = width / 8;
	int numTilesY = height / 8;

	std::pair<int, int> pixelPos(x % 8, y % 8); // Pos. pixel in tile
	std::pair<int, int> tilePos (x / 8, y / 8); // Pos. tile in image

	index = tilePos.second * numTilesX * tileLength + tilePos.first * tileLength;
	index += pixelPos.second * 8 + pixelPos.first;	// Add pos. of pixel inside tile

	return index;
}

std::uint8_t NCGR::getClosestColor(const Color& col, std::vector<Color>& palette) {

	std::uint32_t lowestVal = 196000;
	//std::uint8_t lowestPal = 0;
	std::uint8_t lowestIdx = 0 ;
	for (unsigned int i = 0; i < palette.size(); i++) {
			std::uint8_t dR = palette[i].red - col.red;
			std::uint8_t dB = palette[i].blue - col.blue;
			std::uint8_t dG = palette[i].green - col.green;
			//float dA = palette[i][j].alpha - col.alpha;
			std::uint32_t dist = dR*dR + dB*dB + dG*dG; //SqRt doesn't need to be calculated
			if (dist < lowestVal) {
			//	lowestPal = i / 16;
				lowestIdx = i;
				lowestVal = dist;
			}
	}
	//manually set lowestPal to 0 for now
	return lowestIdx;
}

std::tuple<std::uint8_t, std::vector<std::uint8_t>> NCGR::getClosestColorTile(Tile& t, std::vector<Color>& palette) {
	
	std::uint8_t numPalettes = (std::uint8_t)(palette.size() / 16);
	std::vector<std::uint8_t> matches(numPalettes);
	std::vector<std::vector<std::uint8_t>> indices(numPalettes, std::vector<std::uint8_t>(t.pixels.size(), 0));
	for (unsigned int p = 0; p < t.pixels.size(); p++){
		Color& pix = *t.pixels[p];
		for (unsigned int i = 0; i < palette.size(); i++) {
			if (pix == palette[i]) {
				matches[i / 16]++;
				indices[i / 16][p] = i % 16;
				break;
			}
		}
	}
	//Find best matches
	int bestMatchPal = -1;
	int bestMatchVal = -1;
	for (unsigned int i = 0; i < matches.size(); i++) {
		if (matches[i] > bestMatchVal) {
			bestMatchVal = matches[i];
			bestMatchPal = i;
		}
	}

	//manually set lowestPal to 0 for now
	return std::forward_as_tuple(bestMatchPal, indices[bestMatchPal]);
}

void NCGR::setImgData(std::vector<Tile>& tiles, std::vector<Color>& pal, std::uint16_t w, std::uint16_t h) {
	std::vector<std::tuple<std::uint8_t,  std::vector<std::uint8_t>>> palIdx;
	//std::vector<int>(pal.size());
	for (unsigned int t = 0; t < tiles.size(); t++) {
		Tile& tile = tiles[t];
		palIdx.emplace_back(getClosestColorTile(tile, pal));
	}
	charBlock->setPixelPaletteIdx(palIdx, w, h);
}
//second vector should probably hold both a 16t and another int for palette number
std::tuple<std::vector<Tile>&, std::vector<std::uint16_t>> NCGR::getTiles(std::vector<Color>& allColors) {
	int tWidth = 8;
	int tHeight = 8;


	//Set Default Transparent Tile
	std::vector<std::uint16_t> tileIdx;
	Tile defaultTile;
	for (unsigned u = 0; u < defaultTile.height; u++) {
		for (unsigned v = 0; v < defaultTile.width; v++) {
			int idx = u * 256 + v;
			//cout << idx << "\n";
			auto& p = allColors[idx];
			defaultTile.pixels.emplace_back(&p);
		}
		
	}
	tileIdx.push_back(0);
	tiles.emplace_back(std::move(defaultTile));

	int idx = 1;
	for (unsigned int i = 0; i < 192; i += tHeight) {
		for (unsigned int j = 0; j < 256; j += tWidth) {
			Tile t;
			for (unsigned u = 0; u < t.height; u++) {
				for (unsigned v = 0; v < t.width; v++) {
					int idx = (i + u)*256 + (j + v);
					//cout << idx << "\n";
					auto& p = allColors[idx];
					if (!(p == allColors[0])) {
						t.defaultTile = false;
					}
					t.pixels.emplace_back(&p);
				}
			}
			if (!t.defaultTile) {
				tiles.emplace_back(t);
				tileIdx.push_back(idx++);
			}
			else {
				tileIdx.push_back(0);
			}
		}
	}
	return std::forward_as_tuple(tiles, std::move(tileIdx));
}