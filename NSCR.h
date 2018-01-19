#pragma once

#include "Nitro.h"

#include "NCGR.h"

struct MapInfo {
	std::uint16_t tileIdx;
	std::uint8_t paletteIdx;
	bool flipX;
	bool flipY;
	MapInfo() = default;
	MapInfo(std::byte tIdx) {
		tileIdx = (std::uint32_t)tIdx;
		paletteIdx = 0;
		flipX = false;
		flipY = false;
	}
	MapInfo(std::uint32_t tIdx, std::uint16_t pIdx) {
		tileIdx = (std::uint32_t)tIdx;
		paletteIdx = (std::int8_t)pIdx;
		flipX = false;
		flipY = false;
	}
	MapInfo(std::uint16_t val) {
		tileIdx = (val >> 00) & 0x3FF;
		paletteIdx = (val >> 12) & 0x0F;
		flipX = ((val >> 10) & 0x01) == 1;
		flipY = ((val >> 11) & 0x01) == 1;
	}
	std::byte toByte() {
		return (std::byte)tileIdx;
	}
	std::uint16_t toUInt16() {
		return (std::uint16_t)(
			(tileIdx << 00) |
			(paletteIdx << 12) |
			((flipX ? 1 : 0) << 10) |
			((flipY ? 1 : 0) << 11));
	}

};

class SCRNBlock : public Block {
private:
	std::uint16_t width;
	std::uint16_t height;
	std::uint16_t paletteMode;
	std::uint16_t bgMode;
	std::uint32_t dataSize;

	std::vector<MapInfo> infos;

public:
	SCRNBlock(std::ifstream& iFILE);
	virtual void writeData(std::ofstream& oFILE) final;
	void setMapInfo(const std::vector<Tile>& tiles, const std::vector<std::uint16_t>& palIdx);
	
};


class NSCR : public Nitro {
private:
	SCRNBlock* scrnBlock;
public:
	NSCR(const NSCR& base);
	NSCR(std::string filename);

	void setMapInfo(const std::vector<Tile>& tiles, const std::vector<std::uint16_t>& palIdx);

};