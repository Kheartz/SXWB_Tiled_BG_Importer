#pragma once

#include <iostream>
#include <experimental\filesystem>

#include "CImg.h"
#include "NCLR.h"

using namespace cimg_library;

class Image {
private:
	unsigned int width;
	unsigned int height;
	unsigned int depth;
	std::unique_ptr<CImg<unsigned char>> img;
public:
	Image(std::string filename);
	unsigned int getWidth() const { return width; }
	unsigned int getHeight() const { return height; }
	unsigned char* getData() { return img->data(); }
	unsigned int getDepth() { return img->depth(); }
	Color getPixel(int x, int y) {
		std::uint8_t r = (*img)(x, y, 0);
		std::uint8_t g = (*img)(x, y, 1);
		std::uint8_t b = (*img)(x, y, 2);
		return Color(b, g, r, 0);
	}
};