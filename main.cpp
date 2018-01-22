#include <string>
#include <iostream>
#include <bitset>

#include "Importer.h"
using namespace std;


int main(int argc, char** argv) {
	
	if (argc == 0) {
		std::cout << "./SXW_impexp image.png image.ncgr palette.nclr map.nscr\n";
	}
	else if (argc != 4) {
		std::cout << "Wrong number of arguments\n";
	}

	Importer importer("data/test4.png", "data/BG_NCGR33.bin", "data/BG_NCLR31.bin", "data/BG_NSCR33.bin");
	importer.importFiles();
	return 0;
}