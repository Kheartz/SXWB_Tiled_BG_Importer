#include "Image.h"

Image::Image(std::string filename) {
	std::experimental::filesystem::path convert("C:\\ProgramFiles\\ImageMagick-7.0.7-Q16\\magick.exe");
	cimg::imagemagick_path(convert.string().c_str());
	std::cout << convert.string() << std::endl;
	img = std::make_unique<CImg<unsigned char>>(filename.c_str());
	width = img->width();
	height = img->height();
}