#include "Importer.h"

Importer::Importer(std::string imgfilename, std::string originalNCGRfilename, std::string originalNCLRfilename, std::string originalNSCRfilename) :
	img(imgfilename),
	oriNSCR( originalNSCRfilename ),
	oriNCGR(originalNCGRfilename),
	oriNCLR( originalNCLRfilename )
{

	bool changePalette = true;

	newNCGR = std::make_unique<NCGR>(oriNCGR);
	newNCLR = std::make_unique<NCLR>(oriNCLR, changePalette);
	newNSCR = std::make_unique<NSCR>(oriNSCR);

	Color d1 = img.getPixel(56, 20);
	Color d2 = img.getPixel(57, 20);

	std::vector<Color> allColors;
	std::vector<std::vector<Color>> colorVec(192, std::vector<Color>(256, Color(0,0,0,0)));
	allColors.reserve(img.getWidth() * img.getHeight());

	/*for (unsigned int i = 0; i < img.getHeight(); i++) {
		std::vector<Color> localVec;
		for (unsigned int j = 0; j < img.getWidth(); j++) {
			Color& c = allColors.emplace_back(img.getPixel(j, i));
			localVec.push_back(img.getPixel(j, i));
			//std::cout << c.red << std::endl;
		}
		colorVec.push_back(localVec);
	}*/
	for (unsigned int i = 0; i < img.getHeight(); i++) {
		for (unsigned int j = 0; j < img.getWidth(); j++) {
			Color& c = allColors.emplace_back(img.getPixel(j, i));
			colorVec[i][j] = img.getPixel(j, i);
			//std::cout << c.red << std::endl;
		}
	}
	Color deb = colorVec[20][56];
	Color outp = colorVec[20][57];
	/*for (unsigned int i = 0; i < img.getHeight(); i++) {
		for (unsigned int j = 0; j < img.getWidth(); j++) {
			Color& c = allColors.emplace_back(img.getPixel(i, j));
			//std::cout << c.red << std::endl;
		}
	}*/

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
	if (colors_256.size() > 16) {
		colors_256.insert(colors_256.begin() + 16, colors_256[0]);
	}
	
	while (colors_256.size() < 32) {
		if (colors_256.size() % 16 == 0) {
			colors_256.emplace_back(colors_256[0]);
		} 
		else{
			colors_256.emplace_back(0, 0, 0, 0);
		}
		
	}
	if (changePalette) {
		newNCLR->setPalette(colors_256);
	}
	//palette done
	std::uint16_t mapWidth = ((std::uint16_t)allColors.size() > 256) ? 256 : (std::uint16_t)allColors.size();
	std::uint16_t mapHeight = (std::uint16_t)allColors.size() / mapWidth;

	auto tileSize = std::make_pair<int, int>(8, 8);
	std::uint16_t tileSizeW = mapWidth / tileSize.first;
	std::uint16_t tileSizeH = mapHeight / tileSize.second;

	//image start
	auto[newTiles, tilesIdx] = newNCGR->getTiles(allColors);
	//newNCGR->setImgData(newTiles, allColors, tileSizeW, tileSizeH);
	//newNCGR->setImgData(newTiles, newNCLR->getPalette()[0], tileSizeW, tileSizeH);
	newNCGR->setImgData(newTiles, colors_256, tileSizeW, tileSizeH);
	//image done
	//map start

	//std::vector<std::uint16_t> paletteIdx(tileSizeW * tileSizeH, 0);
	auto& palIdx = newNCGR->getPixelPalIdx();
	//newNSCR->setMapInfo(newTiles, tilesIdx);
	newNSCR->setMapInfo(palIdx, tilesIdx);
	//map done

}

void Importer::importFiles() {

	newNCLR->write("data/test.nclr");
	newNCGR->write("data/test.ncgr");
	newNSCR->write("data/test.nscr");
}