#include "NSCR.h"

SCRNBlock::SCRNBlock(std::ifstream& iFILE) : Block(iFILE) {
	std::streampos blockPos = iFILE.tellg();
	iFILE.read((char*)(&width), sizeof(width));
	iFILE.read((char*)(&height), sizeof(height));
	iFILE.read((char*)(&paletteMode), sizeof(paletteMode));
	iFILE.read((char*)(&bgMode), sizeof(bgMode));
	iFILE.read((char*)(&dataSize), sizeof(dataSize));
	
	std::uint16_t numInfos = (bgMode == 1) ? dataSize : dataSize / 2;

	infos.reserve(numInfos);
	for (int i = 0; i < numInfos; i++) {
		if (bgMode == 1) {
			std::byte temp;
			iFILE.read((char*)(&temp), sizeof(temp));
			infos.emplace_back(temp);
		}
		else {
			std::uint16_t temp;
			iFILE.read((char*)(&temp), sizeof(temp));
			infos.emplace_back(temp);
		}
	}

}

void SCRNBlock::writeData(std::ofstream& oFILE) {
	oFILE.write((char*)(&width), sizeof(width));
	oFILE.write((char*)(&height), sizeof(height));
	oFILE.write((char*)(&paletteMode), sizeof(paletteMode));
	oFILE.write((char*)(&bgMode), sizeof(bgMode));
	oFILE.write((char*)(&dataSize), sizeof(dataSize));

	for (auto& info : infos) {
		if (bgMode == 1) {
			std::byte temp = info.toByte();
			oFILE.write((char*)(&temp), sizeof(temp));
		}
		else {
			std::uint16_t temp = info.toUInt16();
			oFILE.write((char*)(&temp), sizeof(temp));
		}
	}

}

void SCRNBlock::setMapInfo(const std::vector<Tile>& tiles, const std::vector<std::uint16_t>& palIdx) {
	infos.clear();

	//We start with the second palidx since we manually set the first one to be a transparent tile.
	for (std::uint32_t i = 1; i < palIdx.size(); i++) {
		//infos.emplace_back(i, palIdx[0]);
		infos.emplace_back(palIdx[i], 0);
	}
}

NSCR::NSCR(std::string filename) : Nitro(filename) {
	blocks.reserve(numSections);

	blocks.emplace_back(std::make_unique<SCRNBlock>(iFILE));
}

NSCR::NSCR(const NSCR& base) : Nitro(base) {

	if (base.blocks.size() > 0 && base.blocks[0] != nullptr) {
		std::unique_ptr<SCRNBlock> newSCRNBlock = std::make_unique<SCRNBlock>(*(dynamic_cast<SCRNBlock*>(base.blocks[0].get())));
		blocks.emplace_back(std::move(newSCRNBlock));
		scrnBlock = dynamic_cast<SCRNBlock*>(blocks[0].get());
	}
}

void NSCR::setMapInfo(const std::vector<Tile>& tiles, const std::vector<std::uint16_t>& palIdx) {
	scrnBlock->setMapInfo(tiles, palIdx);
}