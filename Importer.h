#pragma once

#include "Image.h"
#include "NCGR.h"
#include "NCLR.h"
#include "NSCR.h"

class Importer {
private:

public:
	Importer(std::string imgfilename, std::string originalNCGRfilename, std::string originalNCLRfilename, std::string originalNSCRfilename);
};