#include "Importer.h"

/*Color getClosestColor(std::const Color& col) {

	std::uint32_t lowestVal = 196000;
	std::pair<int, int> lowestIdx = { 0, 0 };
	for (unsigned int i = 0; i < palette.size(); i++) {
		for (unsigned int j = 0; j < palette[i].size(); j++) {
			std::uint8_t dR = palette[i][j].red - col.red;
			std::uint8_t dB = palette[i][j].blue - col.blue;
			std::uint8_t dG = palette[i][j].green - col.green;
			//float dA = palette[i][j].alpha - col.alpha;
			std::uint32_t dist = dR*dR + dB*dB + dG*dG; //SqRt doesn't need to be calculated
			if (dist < lowestVal) {
				lowestIdx.first = i;
				lowestIdx.second = j;
				lowestVal = dist;
			}
		}
	}
	return palette[lowestIdx.first][lowestIdx.second];
}*/

Importer::Importer(std::string imgfilename, std::string originalNCGRfilename, std::string originalNCLRfilename, std::string originalNSCRfilename) {

	bool changePalette = true;

	Image img(imgfilename);
	NSCR oriNSCR(originalNSCRfilename);
	NCGR oriNCGR(originalNCGRfilename);
	NCLR oriNCLR(originalNCLRfilename);

	NCGR newNCGR(oriNCGR);
	NCLR newNCLR(oriNCLR, changePalette);
	NSCR newNSCR(oriNSCR);

	auto tileSize = std::make_pair<int, int>(8, 8);

	std::vector<Color> allColors;
	allColors.reserve(img.getWidth() * img.getHeight());
	int t = img.getWidth() * img.getHeight();
	for (unsigned int i = 0; i < img.getHeight(); i++) {
		for (unsigned int j = 0; j < img.getWidth(); j++) {
			Color& c = allColors.emplace_back(img.getPixel(j, i));
			//std::cout << c.red << std::endl;
		}
	}

	std::vector<Color> colors_256; //unique Colors
	for (auto& oldC : allColors) {
		bool found = false;
		for (auto& newC : colors_256) {
			if (oldC == newC) {
				found = true;
				break;
			}
		}
		if (!found) {
			colors_256.emplace_back(oldC);
		}
	}
	/*
		FIX LATER: THIS ASSUMES 4BPP 16_16 PALETTES
	*/
	colors_256.insert(colors_256.begin() + 16, colors_256[0]);
	while (colors_256.size() < 32) {
		colors_256.emplace_back(0, 0, 0, 0);
	}
	if (changePalette) {
		newNCLR.setPalette(colors_256);
	}
	//palette done
	std::uint16_t width = ((std::uint16_t)allColors.size() > 256) ? 256 : (std::uint16_t)allColors.size();
	std::uint16_t height = (std::uint16_t)allColors.size() / width;
	/*
	If you make full use of both palettes, you need to match every Tile to a palette
	
	
	std::vector<std::uint32_t> paletteIdx;
	for (unsigned int x = 0; x < width; x += tileSize.first) {
		for (unsigned int y = 0; y < height; y += tileSize.second) {

		}
	}
	But for now, we assume each tile is using the first palette;
	*/
	
	std::uint16_t tileSizeW = width / tileSize.first;
	std::uint16_t tileSizeH = height / tileSize.second;
	std::vector<std::uint32_t> indexedPixels;
	//image start
	
	///newNCGR.getCharBlock()->setWidth(tileSizeW);
	//newNCGR.getCharBlock()->setHeight(tileSizeH);
	//newNCGR.relinearizeData(allColors);
	
	//image done
	auto[newTiles, tilesIdx] = newNCGR.getTiles(allColors);
	newNCGR.setImgData2(newTiles, newNCLR.getPalette()[0], tileSizeW, tileSizeH);
	//newNCGR.setImgData(allColors, newNCLR.getPalette()[0], tileSizeW, tileSizeH);
	//map start

	std::vector<std::uint16_t> paletteIdx(tileSizeW * tileSizeH, 0);
	newNSCR.setMapInfo(newTiles, tilesIdx);
	//map done

	newNCLR.write("data/test.nclr");
	newNCGR.write("data/test.ncgr");
	newNSCR.write("data/test.nscr");
}