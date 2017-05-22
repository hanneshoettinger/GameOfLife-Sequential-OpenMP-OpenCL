# GameOfLife-Sequential-OpenMP-OpenCL

Game of Life performance test simulating a certain amount of life cycles (Sequential vs OpenMP vs OpenCL)

To build and run the program OpenCL is required!

## Usage:

1) Open Console Window in Folder of gol.exe file (.\Executable\Release)
2) type:

		* gol.exe --load "Location of Input File" --save "Location of Output File"
		*  --generations NumberofGenerations --measure 
		*  --mode seq
		*  --mode omp --threads <NUM> 	(standard 8, parallel OpenMP calculation)
		*  --mode ocl 			(parallel OpenCl version)

	OpenCL specific:

		* --device cpu|gpu 	(automatically selects platform & device)
		* --platformID NUM	(user-defined selection)
		* --deviceID NUM		(user-defined selection)
	
3) --measure -> outputs time for program execution
4) check output file


### CONSOLE APPLICATION : GameOfLife Project Overview
-----------------------------------------------------

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
	
		* --help: 		show help for usage
		* --measure:		if set -> output time for execution
		* --generations:		standard 250, set it to any number > 0
		* --mode seq|omp|ocl:	sequential/OpenMP/OpenCL calculation
		* --threads:		OpenMP specific, number of threads
		* --device:		OpenCl specific, cpu|gpu
		* --platformId:		OpenCl specific, selects user-defined platform
		* --deviceId:		OpenCl specific, selects user-defined device

    Mandatory input:
		* --load:		input .gol file
		* --save:		output .gol file
		* --mode:		seq|omp|ocl (with device)

FileIO.cpp
    In the FileIO class you can find the functions for input and output data.
    getFileContents reads the data of the .gol file to a string.
    saveBoardtoFile writes the data to a specified .gol file in the correct format.
    checkBoardSize outputs via reference the board height and width for dynamic memory
    allocation. Checks for the first two numbers in the file, since those are the dimensions.

Timer.cpp
    High Performance Timer for checking program execution time. If argument --measure
    is given, output the time in format 00:00:00.000 in the function stopTimer


### Other notes:

Game of Life implemented logic:

Version 2.0:

Due to performance comparison, the implementation has slightly changed.
NEW:	

	- Board Wrapping:	in the main game loop two additional for loops are implemented to fill the "ghost" columns and rows
				for the wraparound. Therefore if-conditions can be removed for the wrap-checking -> sequential mode
				got a huge performance boost with this implementation -> therefore it is used with OpenMP and OpenCl as well.
				
				in the main function the one-dimensional array gets filled with the board data, leaving the ghost rows/cols empty.
				They are filled in the main game loop for all possible modes respectively.

				The board has now 2 additional cols and rows for the wrapping.
		
	- Pointer Swap:		Only one std::copy is now used to fill the temporary board for calculations, the board is not copied back,
				instead pointer swapping is now implemented.

	- Board access:		Instead of using the #define BOARD( G, X, Y )  ((G)[bwidth*(X)+(Y)]) definition. In the main game loop
				for every iteration an ID is calculated, from where the neighbours are accessed: int ID = (bwidth + 2)*(row)+(col);
	
	- OpenMP:		letsplayMP -> Parallel GoL simulation
				Parallel area: #pragma omp parallel private(count,row,col)
				Parallel loop for nested for-loops: only the outer loop (#pragma omp for)

	- OpenCL:		letsplayCL -> Parallel GoL simulation using OpenCl with the possibility to choose between cpu|gpu
				OpenCl initialization inside the function (possible to output devices: checkOpenCLDevices())
				OpenCl is using Kernels: The simulation is split into three kernels (two for wrapping, one for the game logic)
				The kernel file is called gol-kernels.cl and is built during runtime for execution.
				The "pointer swapping" is here implemented switching between the kernel arguments (input and output buffer)
				For detailed descriptions of the implementation please have a look at the comments in the code

__________________________________________________

Version 1.0:

Classic 3 FOR loop logic -> first loop for the generations, second loop for the rows, third loop for the columns
Create a temp board to write changes to it.
Since wraparound has to be implemented -> first check if we are at first or last row, first or last column
and set help variables, which are added to the array access calculation.
The char array is implemented as a one dimensional array. 
The access is given with: #define BOARD( G, X, Y )  ((G)[bwidth*(X)+(Y)]) to make it more readable. 
Now it is possible to easily add or remove the help variables to get correct field for wraparound.
After that check if dies, stays, or is a new cell and copy the temp board back to the true board.

/////////////////////////////////////////////////////////////////////////////
