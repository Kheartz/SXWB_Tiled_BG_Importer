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


	//data = std::make_unique<std::byte[]>(dataSize);
	//oFILE.write((char*)data.get(), dataSize);

	//Assuming 4bpp for now
	int numGP = 0;
	for (unsigned int i = 0; i < pixelPaletteIdx.size(); i+=2) {
		std::byte b = ((std::byte)pixelPaletteIdx[i] << 4) | ((std::byte)pixelPaletteIdx[i+1]);

		if ((b != (std::byte)0x00)) {
			//cout << bitset<8>(pixelPaletteIdx[i]) << "\n" << bitset<8>(pixelPaletteIdx[i + 1]) << "\n" << bitset<8>((std::uint8_t)b);
			
		}
		//b = std::byte(0x11);
		oFILE.write((char*)(&b), sizeof(std::byte));
		numGP++;
	}
	dataSize = (std::uint32_t)(pixelPaletteIdx.size() / 2);

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

void CHARBlock::setPixelPaletteIdx(std::vector<std::uint8_t>& p, std::uint16_t w, std::uint16_t h){
	tileOrder = 0; 
	width = w;
	height = h;
	pixelPaletteIdx = std::move(p); 

};

NCGR::NCGR(const NCGR& base) : Nitro(base) {

	//for (auto& block : base.blocks) {
	//	blocks.emplace_back(block);
//	}
	if (base.blocks.size() > 0 && base.blocks[0] != nullptr) {
		std::unique_ptr<CHARBlock> newCHARBlock = std::make_unique<CHARBlock>(*(dynamic_cast<CHARBlock*>(base.blocks[0].get())));
		blocks.emplace_back(std::move(newCHARBlock));
		charBlock = dynamic_cast<CHARBlock*>(blocks[0].get());
	}
	/*writeHeader(filename);
	for (auto& block : blocks) {
		block->write(oFILE);
	}
	std::streampos finalPos = oFILE.tellp();
	oFILE.seekp(0x8);
	std::uint32_t finalPosInt = static_cast<std::uint32_t>(finalPos);
	oFILE.write((char*)(&finalPosInt), sizeof(std::uint32_t));
	oFILE.close();*/
}

void NCGR::relinearizeData(std::vector<Color>& pixels) {
	auto[width, height] = charBlock->getWidthAndHeight();
	int length = width * height;
	std::unique_ptr<std::uint32_t[]> original = std::make_unique<std::uint32_t[]>(length);
	for (unsigned int i = 0; i < pixels.size(); i++) {
		std::uint32_t redComponent = pixels[i].red;
		std::uint32_t greenComponent = pixels[i].green;
		std::uint32_t blueComponent = pixels[i].blue;
		std::uint32_t alphaComponent = pixels[i].alpha;

		original[i] = (alphaComponent << 24) | (blueComponent << 16) | (greenComponent << 8) | (redComponent << 0);
	}

	std::unique_ptr<std::uint32_t[]> relinearized = std::make_unique<std::uint32_t[]>(width * height);

	for (int l = 0; l < length; l++) {
		int idx = getIndex(l % width, l / width, width, height);
		relinearized[l] = original[idx];
	}
	std::ofstream yo;
	yo.open("data/wtf.txt", std::ofstream::binary);
	yo.write((char*)relinearized.get(), width*height);
	yo.close();

	int bpp = 4;
	
	std::unique_ptr<std::byte[]> relinearizedBytes = std::make_unique<std::byte[]>(length * bpp / 8); //4 is bpp (fow now)
	int bufferPos = 0;

	for (int i = 0; i < length; i++) {
		std::uint32_t info = original[i] & 0x00FFFFFF;
		for (int s = 0; s < bpp; s++, bufferPos++) {
			std::uint32_t bit = (info >> s) & 1;
			if (bit != 0) {
				int f = 1;
			}

			std::uint32_t dByte = (std::uint32_t)relinearizedBytes[bufferPos / 8];
			dByte |= bit << (bufferPos % 8);
			relinearizedBytes[bufferPos / 8] = (std::byte)dByte;
		}
		//buffer.SetBits(ref bufferPos, this.Format.Bpp(), info);
	}

	charBlock->replaceImageData(relinearizedBytes);
	
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
	std::uint8_t lowestIdx = 0 ;
	for (unsigned int i = 0; i < palette.size(); i++) {
			std::uint8_t dR = palette[i].red - col.red;
			std::uint8_t dB = palette[i].blue - col.blue;
			std::uint8_t dG = palette[i].green - col.green;
			//float dA = palette[i][j].alpha - col.alpha;
			std::uint32_t dist = dR*dR + dB*dB + dG*dG; //SqRt doesn't need to be calculated
			if (dist < lowestVal) {
				lowestIdx = i;
				lowestVal = dist;
			}
	}
	return lowestIdx;
}

/*void NCGR::setImgData(std::vector<Color>& allColors, std::vector<Color>& pal, std::uint16_t w, std::uint16_t h) {
	std::vector<std::uint8_t> palIdx;
	std::vector<int>(pal.size());
	for (auto& c : allColors) {
		palIdx.emplace_back(getClosestColor(c, pal));
	}
	charBlock->setPixelPaletteIdx(palIdx, w, h);
}*/
void NCGR::setImgData2(std::vector<Tile>& tiles, std::vector<Color>& pal, std::uint16_t w, std::uint16_t h) {
	std::vector<std::uint8_t> palIdx;
	std::vector<int>(pal.size());
	for (auto& t : tiles) {
		//for (auto p : t.pixels) {
		for (unsigned int i = 0; i < 8; i++) {
			for (unsigned int j = 0; j < 8; j++) {
				auto& p = t.pixels[i * 8 + j];
				palIdx.emplace_back(getClosestColor(*p, pal));
			}
			
		}
	}
	charBlock->setPixelPaletteIdx(palIdx, w, h);
}
//second vector should probably hold both a 16t and another int for palette number
std::tuple<std::vector<Tile>&, std::vector<std::uint16_t>> NCGR::getTiles(std::vector<Color>& allColors) {
	int tWidth = 8;
	int tHeight = 8;

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