#include "NCLR.h"


//First 8 bytes of the block have already been open and read.
PLTTBlock::PLTTBlock(std::ifstream& iFILE) : Block(iFILE) {
	std::streampos blockPos = iFILE.tellg();
	iFILE.read((char*)(&depth), sizeof(depth)); //0x8
	iFILE.read((char*)(&extendedPalette), sizeof(extendedPalette)); //0xa
	std::uint32_t garbage;
	iFILE.read((char*)(&garbage), sizeof(garbage)); //0xa
	iFILE.read((char*)(&paletteOffset), sizeof(paletteOffset)); //0xc

	adjustedSize = size - 0x8 - 0x10;

	paletteSize = adjustedSize;
	paletteData = std::make_unique<std::uint16_t[]>(paletteSize);
	iFILE.read((char*)(paletteData.get()), adjustedSize);
}
PLTTBlock::PLTTBlock(const PLTTBlock& other) : Block(other){
	depth = other.depth;
	extendedPalette = other.extendedPalette;
	adjustedSize = other.adjustedSize;
	paletteOffset = other.paletteOffset;

	paletteSize = other.paletteSize;

	paletteData = std::make_unique<std::uint16_t[]>(paletteSize);
	std::memcpy(paletteData.get(), other.paletteData.get(), paletteSize);
}

void PLTTBlock::changePaletteData(const std::uint16_t* originalPData, std::uint32_t size) {
	paletteData = std::make_unique<std::uint16_t[]>(size);
	std::memcpy(paletteData.get(), originalPData, size);
}
void PLTTBlock::writeData(std::ofstream& oFILE) {

	oFILE.write((char*)(&depth), sizeof(depth));
	oFILE.write((char*)(&extendedPalette), sizeof(extendedPalette));
	oFILE.write((char*)(&paletteSize), sizeof(paletteSize));
	std::uint32_t temp = 0x10;
	oFILE.write((char*)(&temp), sizeof(temp));
	oFILE.write((char*)(paletteData.get()), paletteSize);
}
PCMPBlock::PCMPBlock(std::ifstream& iFILE) : Block(iFILE) {
	
	iFILE.read((char*)(&numPalettes), sizeof(numPalettes));
	iFILE.read((char*)(&constant), sizeof(constant));
	iFILE.read((char*)(&dataOffset), sizeof(dataOffset));

	paletteIndex = std::make_unique<std::uint16_t[]>(numPalettes);

	for (int i = 0; i < numPalettes; i++)
		iFILE.read((char*)(&paletteIndex[i]), sizeof(std::uint16_t));
}
PCMPBlock::PCMPBlock(const PCMPBlock& other) : Block(other) {
	numPalettes = other.numPalettes;
	constant = other.constant;
	dataOffset = other.dataOffset;

	paletteIndex = std::make_unique<std::uint16_t[]>(numPalettes);
	std::memcpy(paletteIndex.get(), other.paletteIndex.get(), sizeof(std::uint16_t) * numPalettes);
}
void PCMPBlock::writeData(std::ofstream& oFILE) {
	oFILE.write((char*)(&numPalettes), sizeof(numPalettes));
	oFILE.write((char*)(&constant), sizeof(constant));
	std::uint32_t temp = 0x08;
	oFILE.write((char*)(&temp), sizeof(temp));

	for (int i = 0; i < numPalettes; i++)
		oFILE.write((char*)(&paletteIndex[i]), sizeof(paletteIndex[i]));
}
NCLR::NCLR(std::string filename) : Nitro(filename) {
	blocks.reserve(numSections);

	auto& pltt = blocks.emplace_back(std::make_unique<PLTTBlock>(iFILE));
	palBlk = dynamic_cast<PLTTBlock*>(pltt.get());

	auto& pcmp = blocks.emplace_back(std::make_unique<PCMPBlock>(iFILE));

	int numColors = (dynamic_cast<PLTTBlock*>(pltt.get())->getDepth() == ColorFormat::Indexed_8bpp) ? 0x100 : 0x10;
	int numPalettes = 0;

	std::uint16_t* index = dynamic_cast<PCMPBlock*>(pcmp.get())->getIndex();

}
/*
@param 1 (base): Base NCLR file. Used to extract previous information for a new NCLR file

All of the block information can be copied except for:
The palette data. We will extract from the image.
The palette size data. Depends on the number of colors of the new palette.
*/
NCLR::NCLR(const NCLR& base, bool changePalette) : Nitro(base) {
	if (base.blocks.size() > 0 && base.blocks[0] != nullptr) {
		std::unique_ptr<PLTTBlock> newPLTTBlock = std::make_unique<PLTTBlock>(*(dynamic_cast<PLTTBlock*>(base.blocks[0].get())));
		blocks.emplace_back(std::move(newPLTTBlock));
		palBlk = dynamic_cast<PLTTBlock*>(blocks[0].get());
		if (changePalette) {
			palBlk->changePaletteData(nullptr, 0);
		}
	}
	if (base.blocks.size() > 1 && base.blocks[1] != nullptr) {
		std::unique_ptr<PCMPBlock> newPCMPBlock = std::make_unique<PCMPBlock>(*(dynamic_cast<PCMPBlock*>(base.blocks[1].get())));
		blocks.emplace_back(std::move(newPCMPBlock));
	}
	
}

std::tuple<std::uint16_t*, std::uint32_t> PLTTBlock::getPaletteData(){
	return std::forward_as_tuple(paletteData.get(), paletteSize);
}

std::uint16_t PLTTBlock::getBGR555(Color& c) {
	//000000000011111 = 0x001F
	//000001111100000 = 0x03C0
	//111110000000000 = 0x7C00
	//Final bit unused.
	//Have NO idea why multiplied by 0x08....Had to reference Tinke for this.
	std::uint16_t redComponent = std::uint16_t(c.red/8) & 0x001F;
	std::uint16_t greenComponent = (std::uint16_t(c.green / 8) << 5)  & 0x03E0;
	std::uint16_t blueComponent = (std::uint16_t(c.blue / 8) << 10)  & 0x7C00;
	return (redComponent | greenComponent | blueComponent);
}
void PLTTBlock::populatePaletteData(std::vector<std::vector<Color>>& colorSet) {
	int numcolors = 0;
	for (auto& pal : colorSet) {
		numcolors += pal.size();
	}
	paletteData = std::make_unique<std::uint16_t[]>(numcolors);

	int idx = 0;
	for (auto& pal : colorSet) {
		for (auto& color : pal) {
			std::uint16_t newFormattedColor = getBGR555(color);
			paletteData[idx++] = newFormattedColor;
		}
	}
	paletteSize = numcolors * 2;
}
void NCLR::setPalette(std::vector<Color>& pal) {
	palette.clear();
	unsigned int totalColors = pal.size();
	if (totalColors > 256) {
		//Throw error
	}
	palette.resize(totalColors % 16);
	int currPalette = 0;
	palette.resize(16);
	palette[currPalette].reserve(16);
	int idx = 0;
	for (unsigned int i = 0; i < pal.size(); i++) {
		palette[currPalette].emplace_back(pal[i]);
		idx++;
		if (idx >= 16) {
			currPalette++;
			idx = 0;
		}
	}
	if (palBlk != nullptr) {
		palBlk->populatePaletteData(palette);
	}
}
void NCLR::setPalette(std::vector<std::vector<Color>>& pal) {
	palette.clear();
	palette = pal;
}
