NAME: Hoettinger Hannes
Studienkennzahl: 1510585004

Usage:

1) Open Console Window in Folder of gol.exe file (x64/Release)
2) type:
	gol --load "Location of Input File" --save "Location of Output File" --generations NumberofGenerations --measure --mode seq
3) --measure is not necessary, if typed in -> outputs time for program execution
4) check output file

========================================================================
    CONSOLE APPLICATION : GameOfLife Project Overview
========================================================================

This file contains a summary of what you will find in each of the files that
make up your GameOfLife application.

For detailed code information please check the comments in the source files.


GameOfLife.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

GameOfLife.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

GameOfLife.cpp
    This is the main application source file.
    In the main function the user input is checked and then the function getFileContents
    is called to read the input string data to boardStr. The boardStr is checked for the
    current size with checkBoardSize of the Class FileIO.
    Create dynamic char array for the board and fill with 0/1 for faster processing.
    
    Call letsplay -> executes the GoL logic with the given board and generations.

    Finally save the result of GoL with saveBoardFile to the outputfile.

InputParser.cpp
    Checks the input arguments in the command line.
    Possible input:
		--help: 	show help for usage
		--measure:	if set -> output time for execution
		--mode seq:	for future update	
		--generations:	standard 250, set it to any number > 0
    Mandatory input:
		--load:		input .gol file
		--save:		output .gol file

FileIO.cpp
    In the FileIO class you can find the functions for input and output data.
    getFileContents reads the data of the .gol file to a string.
    saveBoardtoFile writes the data to a specified .gol file in the correct format.
    checkBoardSize outputs via reference the board height and width for dynamic memory
    allocation. Checks for the first two numbers in the file, since those are the dimensions.

Timer.cpp
    High Performance Timer for checking program execution time. If argument --measure
    is given, output the time in format 00:00:00.000 in the function stopTimer

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named GameOfLife.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

Game of Life implemented logic:

Classic 3 for loop logic -> first loop for the generations, second loop for the rows, third loop for the columns
Create a temp board to write changes to it.
Since wraparound has to be implemented -> first check if at first or last row, first or last column
and set help variables, which are added to the array access calculation.
The char array is implemented as a one dimensional array. 
The access is given with: #define BOARD( G, X, Y )  ((G)[bwidth*(X)+(Y)])
to make it more readable. Now it is possible to easily add or remove the help variables to get correct field for wraparound.
After that check if dies, stays, or is a new cell and copy the temp board back to the true board.

/////////////////////////////////////////////////////////////////////////////
