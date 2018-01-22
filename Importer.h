#pragma once

#include "Image.h"
#include "NCGR.h"
#include "NCLR.h"
#include "NSCR.h"

class Importer {
private:
	Image img;
	NSCR oriNSCR;
	NCGR oriNCGR;
	NCLR oriNCLR;


	std::unique_ptr<NCGR> newNCGR;
	std::unique_ptr<NCLR> newNCLR;
	std::unique_ptr<NSCR> newNSCR;

public:
	Importer(std::string imgfilename, std::string originalNCGRfilename, std::string originalNCLRfilename, std::string originalNSCRfilename);
	void importFiles();
};