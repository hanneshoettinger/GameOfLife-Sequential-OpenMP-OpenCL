#pragma once
class FileIO
{
public:
	FileIO();
	~FileIO();

	// static to call without object
	// input file from .gol file 
	static std::string getFileContents(); //very fast!! 

	static void saveBoardtoFile(char *boardArr, int &bheight, int &bwidth);
	
	static void checkBoardSize(std::string &board1, int &bheight, int &bwidth);

};

