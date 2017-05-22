#include "stdafx.h"
#include "FileIO.h"
#include "InputParser.h"

FileIO::FileIO()
{
}


FileIO::~FileIO()
{
}

// stream input to string from .gol file
std::string FileIO::getFileContents()
{
	std::ifstream in(inputFile, std::ios::in | std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

// save result of generations to outputfile .gol
void FileIO::saveBoardtoFile(char *boardArr, int &bheight, int &bwidth)
{
	std::ofstream myfile;
	myfile.open(outputFile);

	myfile << bwidth << "," << bheight << "\n";

	for (int rr = 1; rr <= bheight; rr++) {
		for (int cc = 1; cc <= bwidth; cc++) {
			if (boardArr[rr * (bwidth + 2) + cc] == 0) {
				myfile << '.';
			}
			else {
				myfile << 'x';
			}
		}
		myfile << "\n";
	}

	myfile.close();
}

// check Board Size
void FileIO::checkBoardSize(std::string &boardStr, int &bheight, int &bwidth)
{
	int xx = 0;
	std::string start, end;

	// check size specs
	for (int ii = 0; ii <= boardStr.size(); ii++)
	{
		if (boardStr[ii] == ',') {
			xx = ii + 1;
			for (int tt = 0; tt < ii; tt++) {
				start += boardStr[tt];
			}
		}
		else if (boardStr[ii] == '.' | boardStr[ii] == 'x') {
			for (int tt = xx; tt < ii; tt++) {
				end += boardStr[tt];
			}
			break;
		}
	}

	// set width and height of the board
	bwidth = atoi(start.c_str());
	bheight = atoi(end.c_str());
}