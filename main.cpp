#include <string>
#include <iostream>
#include <bitset>

#include "Importer.h"
using namespace std;


int main() {
	Importer imp("data/test.png", "data/BG_NCGR33.bin", "data/BG_NCLR31.bin", "data/BG_NSCR33.bin");
	return 0;
}