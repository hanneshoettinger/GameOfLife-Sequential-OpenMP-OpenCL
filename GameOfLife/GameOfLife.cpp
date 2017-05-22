#include "stdafx.h"
#include "Timer.h"
#include "InputParser.h"
#include "FileIO.h"

// Simplifying access to grid elements
//#define BOARD( G, Y, X )  ((G)[(bwidth)*(Y)+(X)])

#define LOCAL_SIZE 10

// Prototypes for GoL (1st: sequential, 2nd: OpenMP, 3rd: OpenCL)
void letsplay(char* inmat, int generations);
void letsplayMP(char* inmat, int generations);
void letsplayCL(char* inmat, int generations);

// OpenCL Device Check with console output
void checkOpenCLDevices();

// Global Values
int bheight = 0;
int bwidth = 0;

// Timer Class
TimerFunc timer;

cl_platform_id cpPlatform;        // OpenCL platform
cl_device_id device_id;           // device ID
cl_context context;               // context
cl_command_queue queue;           // command queue
cl_program program;               // program

// OpenCL Buffers to hold board
cl_mem inmat_cl;
cl_mem newmat_cl;

int main(int argc, char *argv[])
{
		// INIT -> check Input Arguments and FileIO****
		timer.startTimer();
		
		int check = checkInputs(argc, argv);
		if (check == 0) { return 0; }
		// input file from .gol file 
		std::string boardStr = FileIO::getFileContents();
		// checks board size to allocate memory of char array -> scans for the parameter in board1
		FileIO::checkBoardSize(boardStr, bheight, bwidth); // call by reference

		// create board array
		char *boardArr = new char[bwidth*bheight]; 

		// set the parameters in boardArr to 0 and 1 for easy calculations
		// fill boardArr from input stream of boardStr -> C++ 11 loop
		int counter = 0;
		for (char& c : boardStr) {
			if (c == '.'){
				boardArr[counter] = 0;
				counter++;
			}
			else if (c == 'x'){
				boardArr[counter] = 1;
				counter++;
			}
		}

		// create second board array and add ghost rows/columns for wrap around
		char *boardArr2 = new char[(bwidth + 2)*(bheight + 2)];

		// fill board for wrap around
		int row ,col;
		for (row = 1; row <= bheight; row++) {
			for (col = 1; col <= bwidth; col++) {
				boardArr2[row*(bwidth + 2) + col] = boardArr[(row-1)*(bwidth) + (col-1)];
			}
		}	

		timer.stopTimer(measureOn);
		//*********************************************
		
		// start Game of Life *************************
		timer.startTimer();
		if		(Mode == "seq") { letsplay(boardArr2, generations); }
		else if (Mode == "omp") { letsplayMP(boardArr2, generations); }
		else if (Mode == "ocl") { letsplayCL(boardArr2, generations); }
		timer.stopTimer(measureOn);
		//*********************************************
		
		// save result to file ************************
		timer.startTimer();
		
		FileIO::saveBoardtoFile(boardArr2, bheight, bwidth);

		//*********************************************
		// clear arrays
		delete[] boardArr;
		delete[] boardArr2;

		timer.stopTimer(measureOn);
		std::cout << std::endl;

		timer.~TimerFunc();

	return 0;
}

//Sequential Game of life simulation with given board and amount of generations
void letsplay(char* inmat, int generations)
{
	// create tempmatrix to check next generation
	char* newmat;
	newmat = new char[(bwidth + 2)*(bheight + 2)];

	// neighbour counter
	int count = 0;
	int row, col;

	// copy inmat to tempmat
	std::copy(inmat, (inmat + (bwidth + 2)*(bheight + 2)), newmat);

	for (int curgen = 0; curgen < generations; curgen++)
	{	
		// Wrap the Board and fill "ghost" lines *****************************************
		// Left-Right columns
		for (row = 1; row <= bheight; row++) {
			inmat[row*(bwidth + 2) + bwidth + 1] = inmat[row*(bwidth + 2) + 1]; 
			inmat[row*(bwidth + 2)] = inmat[row*(bwidth + 2) + bwidth]; 
		}

		// Top-Bottom rows
		for (col = 0; col <= bwidth + 1; col++) {
			inmat[(bwidth + 2)*(bheight + 1) + col] = inmat[(bwidth + 2) + col]; 
			inmat[col] = inmat[(bwidth + 2)*bheight + col]; 
		}

		// Execute Game Logic only on the "inner" board **********************************
		for (row = 1; row <= bheight; row++) {
			for (col = 1; col <= bwidth; col++) {
				count = 0;	

				int ID = (bwidth + 2)*(row)+(col);

				// Get the number of neighbors for a given grid point
				count = inmat[ID + (bwidth + 2)]
					+ inmat[ID - (bwidth + 2)]
					+ inmat[ID + 1]
					+ inmat[ID - 1]          
					+ inmat[ID + (bwidth + 3)]
					+ inmat[ID - (bwidth + 3)] 
					+ inmat[ID - (bwidth + 1)]
					+ inmat[ID + (bwidth + 1)];

				//DIE
				if (count < 2 || count > 3) {newmat[ID] = 0; }
				//STAY
				else if (count == 2) { newmat[ID] = inmat[ID]; }
				//NEW
				else if (count == 3) { newmat[ID] = 1; }
				
			}
		}

		// pointer swap -> faster than copying the whole array...
		char *temp = newmat;
		newmat = inmat;
		inmat = temp;

		// copy back to inmat
		//std::copy(newmat, (newmat + (bwidth + 2)*(bheight + 2)), inmat);
	}
	delete[] newmat;
}

//OpenMP Game of life simulation with given board and amount of generations
void letsplayMP(char* inmat, int generations)
{
	// create tempmatrix to check next generation
	char* newmat;
	newmat = new char[(bwidth + 2)*(bheight + 2)];

	// neighbour counter
	int count = 0;
	int row, col;

	// copy inmat to tempmat
	std::copy(inmat, (inmat + (bwidth + 2)*(bheight + 2)), newmat);

	for (int curgen = 0; curgen < generations; curgen++)
	{
		// set number of threads and perform parallel for loop
		omp_set_num_threads(numThreads);
		// start parallel mode with private variables
		#pragma omp parallel private(count,row,col)
		{

			// Wrap the Board and fill "ghost" lines *****************************************
			// Left-Right columns
			for (row = 1; row <= bheight; row++) {
				inmat[row*(bwidth + 2) + bwidth + 1] = inmat[row*(bwidth + 2) + 1]; //Copy first real column to right most ghost column
				inmat[row*(bwidth + 2)] = inmat[row*(bwidth + 2) + bwidth]; //Copy last real column to left most ghost column
			}

			// Top-Bottom rows
			for (col = 0; col <= bwidth + 1; col++) {
				inmat[(bwidth + 2)*(bheight + 1) + col] = inmat[(bwidth + 2) + col]; //Copy first real row to bottom ghost row
				inmat[col] = inmat[(bwidth + 2)*bheight + col]; //Copy last real row to top ghost row
			}

			// parallel for loop using OpenMP in parallel area for the row-loop
			// Execute Game Logic only on the "inner" board **********************************
			#pragma omp for 
			for (row = 1; row <= bheight; row++) {
				for (col = 1; col <= bwidth; col++) {
					count = 0;

					int ID = (bwidth + 2)*(row)+(col);

					// Get the number of neighbors for a given grid point
					count = inmat[ID + (bwidth + 2)]
						+ inmat[ID - (bwidth + 2)]
						+ inmat[ID + 1]
						+ inmat[ID - 1]
						+ inmat[ID + (bwidth + 3)]
						+ inmat[ID - (bwidth + 3)]
						+ inmat[ID - (bwidth + 1)]
						+ inmat[ID + (bwidth + 1)];

					//DIE
					if (count < 2 || count > 3) { newmat[ID] = 0; }
					//STAY
					else if (count == 2) { newmat[ID] = inmat[ID]; }
					//NEW
					else if (count == 3) { newmat[ID] = 1; }

				}
			}
		}

		// pointer swap -> faster than copying the whole array...
		char *temp = newmat;
		newmat = inmat;
		inmat = temp;

		// copy back to inmat
		//std::copy(newmat, (newmat + (bwidth + 2)*(bheight + 2)), inmat);
	}
	delete[] newmat;
}

//OpenCL Game of life simulation with given board and amount of generations
void letsplayCL(char* inmat, int generations)
{
	cl_int err;

	cl_uint numPlatforms;
	cl_platform_id* platformIds;
	cl_uint numDevices;
	cl_device_id* deviceIds = NULL;

	// check the available devices for OpenCL
	//checkOpenCLDevices();

	// ******************************************* Device Selection: cpu or gpu/PlatformID,DeviceID ********************************
	err = clGetPlatformIDs(0, NULL, &numPlatforms);

	//std::cout << "Number of platforms: " << numPlatforms << std::endl;
	platformIds = (cl_platform_id*)alloca(sizeof(cl_platform_id)*numPlatforms);
	err = clGetPlatformIDs(numPlatforms, platformIds, NULL);
	if (err != CL_SUCCESS) {
		std::cout << "Error: Failed to get Platform IDs\n";
	}

	// automatically select platform and device
	if (sel_device == "cpu" | sel_device == "gpu")
	{
		// Bind to platform
		err = clGetPlatformIDs(numPlatforms, platformIds, NULL);
		if (err != CL_SUCCESS) {
			std::cout << "Error: Failed to find a platform\n";
		}
		if (sel_device == "cpu") {
			// Get ID for the device
			err = clGetDeviceIDs(platformIds[0], device_type, 1, &device_id, NULL);
			if (err != CL_SUCCESS) {
				err = clGetDeviceIDs(platformIds[1], device_type, 1, &device_id, NULL);
				if (err != CL_SUCCESS) {
					std::cout << "Error: Failed to create a device group\n";
				}
			}
		}
		else if (sel_device == "gpu") {
			// Get ID for the device
			err = clGetDeviceIDs(platformIds[0], device_type, 1, &device_id, NULL);
			if (err != CL_SUCCESS) {
				err = clGetDeviceIDs(platformIds[1], device_type, 1, &device_id, NULL);
				if (err != CL_SUCCESS) {
					std::cout << "Error: Failed to create a device group\n";
				}
			}
		}
	}
	// if no device is selected then the platform and device is manually selected
	else
	{
		err = clGetDeviceIDs(platformIds[plID], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);

		//std::cout << "Number of devices ";std::cout << numDevices << std::endl;
		deviceIds = (cl_device_id*)alloca(sizeof(cl_platform_id)*numDevices);

		// Get ID for the device
		err = clGetDeviceIDs(platformIds[plID], CL_DEVICE_TYPE_ALL, numDevices, deviceIds, NULL);
		if (err != CL_SUCCESS) {
			std::cout << "Error: Failed to create a device group\n";
		}

		// select device
		device_id = deviceIds[dvID];
	}

	// ******************************************* OpenCL Init Process -> create necessary structure ********************************
	// Create a context
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	if (!context) {
		std::cout << "Error: Failed to create a compute context\n";
	}
	
	// cl_queue_properties props[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
	// Create a command queue
	queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
	// only OpenCL 2.0 is using with properties...not supported with OpenCL 1.2 -> see stdafx.h -> use deprecated functions in OpenCL 2.0
	// queue = clCreateCommandQueueWithProperties(context, device_id, props, &err);
	if (!queue) {
		std::cout << "Error: Failed to create a command commands\n";
	}
	// load opencl kernel code and create a program with that source file
	std::ifstream cl_file("gol-kernels.cl"); // -> OpenCL Kernel Source File -> Game of Life logic is implemented here
	std::string cl_string(std::istreambuf_iterator<char>(cl_file), (std::istreambuf_iterator<char>()));
	const char *cl_source = cl_string.c_str();
	program = clCreateProgramWithSource(context, 1, &cl_source, NULL, &err);
	if (!program) {
		std::cout << "Error: Failed to create compute program\n";
	}

	// Build the program executable
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		size_t len;
		char buffer[2048];

		std::cout << "Error: Failed to build program executable!\n";
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		printf("%s\n", buffer);  // print build info for debug purpose
	}

	// ******************************************* Create the Kernels which are specified in the .cl file ********************************
	// Create the GOL kernel in the program we wish to run
	cl_kernel k_gol = clCreateKernel(program, "GOL", &err);
	if (!k_gol || err != CL_SUCCESS) {
		std::cout << "Error: Failed to create GOL kernel \n";
	}

	// Create the col kernel in the program
	cl_kernel k_wrapcols = clCreateKernel(program, "wrapcols", &err);
	if (!k_wrapcols || err != CL_SUCCESS) {
		std::cout << "Error: Failed to create WrapCols kernel \n";
	}

	// Create the row kernel in the program
	cl_kernel k_wraprows = clCreateKernel(program, "wraprows", &err);
	if (!k_wraprows || err != CL_SUCCESS) {
		std::cout << "Error: Failed to create WrapRows kernel \n";
	}

	// release program
	clReleaseProgram(program);

	// ******************************************* Create and set the OpenCL Buffers to hold the board ********************************
	// Create the input and output arrays in device memory 
	size_t boardsize = sizeof(char)*((bwidth + 2)*(bheight + 2));
	inmat_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, boardsize, NULL, NULL);
	newmat_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, boardsize, NULL, NULL);
	if (!inmat_cl || !newmat_cl)
	{
		std::cout << "Error: Failed to allocate device memory!\n";
	}
	
	// write the board to the device memory buffer
	err = clEnqueueWriteBuffer(queue, inmat_cl, CL_TRUE, 0, boardsize, (inmat), 0, NULL, NULL);  
	if (err != CL_SUCCESS){std::cout << "Error: Failed to write to source array!\n";}

	// Set the arguments to GOL  kernel
	err = clSetKernelArg(k_gol, 0, sizeof(int), &bheight);
	err |= clSetKernelArg(k_gol, 1, sizeof(int), &bwidth);
	err |= clSetKernelArg(k_gol, 2, sizeof(cl_mem), &inmat_cl);
	err |= clSetKernelArg(k_gol, 3, sizeof(cl_mem), &newmat_cl);
	if (err != CL_SUCCESS) {
		std::cout << "Error: Failed to set kernel arguments\n";
	}

	// Set the arguments to cols kernel
	err = clSetKernelArg(k_wrapcols, 0, sizeof(int), &bheight);
	err |= clSetKernelArg(k_wrapcols, 1, sizeof(int), &bwidth);
	err |= clSetKernelArg(k_wrapcols, 2, sizeof(cl_mem), &inmat_cl);
	if (err != CL_SUCCESS) {
		std::cout << "Error: Failed to set kernel arguments\n";
	}

	// Set the arguments to rows kernel
	err = clSetKernelArg(k_wraprows, 0, sizeof(int), &bheight);
	err |= clSetKernelArg(k_wraprows, 1, sizeof(int), &bwidth);
	err |= clSetKernelArg(k_wraprows, 2, sizeof(cl_mem), &inmat_cl);
	if (err != CL_SUCCESS) {
		std::cout << "Error: Failed to set kernel arguments\n";
	}

	// ******************************************* Define the work-group-sizes for OpenCL execution ********************************

	// Set kernel local and global sizes
	size_t RowGlobalSize, ColGlobalSize, LocalSize;
	LocalSize = LOCAL_SIZE;
	// Calculate number of total work items in respect of the LocalSize (must be a multiple of it...) 
	// in this case it is possible to set only bheight, since we set Local Size to a multiple of the board dimensions
	ColGlobalSize = (size_t)(bheight);
	// row global size has two more elements for the wrapping -> therefore a if statement is implemented in the kernel to avoid access violation
	// this method was also tested using automatic setting of the LocalSize -> was very slow
	RowGlobalSize = (size_t)ceil((bwidth + 2) / (float)LocalSize)*LocalSize;

	// two dimensional Local Size for Game of Life since it has to loop over Row and Col
	size_t GolLocalSize[2] = { LOCAL_SIZE, LOCAL_SIZE };
	// Global Size of row and col is basically the boardsize...
	size_t rowGlobal = (size_t)(bheight);
	size_t colGlobal = (size_t)(bwidth);
	// two dimensional for Game of Life Kernel -> loop over row and col
	size_t GolGlobalSize[2] = { rowGlobal, colGlobal };

	// define an offset for k_gol kernel and k_wrapcols Kernel to start automatically from array pos 1; rowoffset = 0 -> start from 0
	const size_t offset[2] = { 1, 1 };
	const size_t coloffset = 1;

	// ******************************************* Start the Main Game Loop for the simulation ********************************
	for (int curgen = 0; curgen < generations; curgen++) {

		// execute Kernels in correct order -> first wrap cols then rows -> execute board logic with wrapper
		// it is also possible to set work-group-size = LocalSize to NULL -> choose automatically the size (faster if already set in this implementation...)
		err = clEnqueueNDRangeKernel(queue, k_wrapcols, 1, &coloffset, &ColGlobalSize, &LocalSize, 0, NULL, NULL);
		err |= clEnqueueNDRangeKernel(queue, k_wraprows, 1, NULL, &RowGlobalSize, &LocalSize, 0, NULL, NULL);
		err |= clEnqueueNDRangeKernel(queue, k_gol, 2, offset, GolGlobalSize, GolLocalSize, 0, NULL, NULL);

		// instead of copy Buffer it is also possible to switch arguments in the Kernels of newmat and inmat
		if (curgen % 2 == 1) {
			err |= clSetKernelArg(k_gol, 2, sizeof(cl_mem), &inmat_cl);
			err |= clSetKernelArg(k_gol, 3, sizeof(cl_mem), &newmat_cl);
			err |= clSetKernelArg(k_wrapcols, 2, sizeof(cl_mem), &inmat_cl);
			err |= clSetKernelArg(k_wraprows, 2, sizeof(cl_mem), &inmat_cl);
		}
		else {
			err |= clSetKernelArg(k_gol, 2, sizeof(cl_mem), &newmat_cl);
			err |= clSetKernelArg(k_gol, 3, sizeof(cl_mem), &inmat_cl);
			err |= clSetKernelArg(k_wrapcols, 2, sizeof(cl_mem), &newmat_cl);
			err |= clSetKernelArg(k_wraprows, 2, sizeof(cl_mem), &newmat_cl);
		}
		
		// Copy the output = newmat_cl to the input = inmat_cl for the next iteration
		//err |= clEnqueueCopyBuffer(queue, newmat_cl, inmat_cl, 0, 0, boardsize, 0, NULL, NULL);

	}

	if (err != CL_SUCCESS) {
		printf("Error: Failed in game loop %d\n", err);
	}

	// Wait for the command queue to get serviced before reading back results
	clFinish(queue);

	// Read the results from the device and store it back to our board Array inmat
	err = clEnqueueReadBuffer(queue, inmat_cl, CL_TRUE, 0, boardsize, inmat, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to read output array\n");
	}

	// Shutdown and cleanup
	clReleaseMemObject(inmat_cl);
	clReleaseMemObject(newmat_cl);
	//clReleaseProgram(program);
	clReleaseKernel(k_wrapcols);
	clReleaseKernel(k_wraprows);
	clReleaseKernel(k_gol);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
	// -> back to main, with the final board stored in inmat
}

// Function to check available OpenCL Devices
void checkOpenCLDevices()
{
	cl_int err;

	cl_uint numPlatforms;
	cl_platform_id* platformIds;
	cl_uint numDevices;
	cl_device_id* deviceIds = NULL;

	err = clGetPlatformIDs(0, NULL, &numPlatforms);

	std::cout << "Number of platforms: " << numPlatforms << std::endl;
	platformIds = (cl_platform_id*)alloca(sizeof(cl_platform_id)*numPlatforms);
	err = clGetPlatformIDs(numPlatforms, platformIds, NULL);
	for (unsigned int i = 0; i < numPlatforms; i++) {
		std::cout << "PLATFORM:" << i << std::endl;
		std::cout << "Platform ID "; std::cout << platformIds[i] << std::endl;
		cl_platform_id id = platformIds[i];
		size_t size;
		err = clGetPlatformInfo(id, CL_PLATFORM_VENDOR, 0, NULL, &size);

		char* vendor = (char*)alloca(sizeof(char)*size);
		err = clGetPlatformInfo(id, CL_PLATFORM_VENDOR, size, vendor, NULL);

		std::cout << "Vendor name "; std::cout << vendor << std::endl;

		err = clGetPlatformInfo(id, CL_PLATFORM_NAME, 0, NULL, &size);

		char* pname = (char*)alloca(sizeof(char)*size);
		err = clGetPlatformInfo(id, CL_PLATFORM_NAME, size, pname, NULL);

		std::cout << "Platform name "; std::cout << pname << std::endl;

		err = clGetPlatformInfo(id, CL_PLATFORM_VERSION, 0, NULL, &size);

		char* version = (char*)alloca(sizeof(char)*size);
		err = clGetPlatformInfo(id, CL_PLATFORM_VERSION, size, version, NULL);

		std::cout << "Platform version "; std::cout << version << std::endl;

		err = clGetDeviceIDs(id, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);

		std::cout << "Number of devices "; std::cout << numDevices << std::endl;
		deviceIds = (cl_device_id*)alloca(sizeof(cl_platform_id)*numDevices);
		err = clGetDeviceIDs(id, CL_DEVICE_TYPE_ALL, numDevices, deviceIds, NULL);

		err = clGetPlatformInfo(id, CL_PLATFORM_EXTENSIONS, 0, NULL, &size);
		char* extensions = (char*)alloca(sizeof(char)*size);
		err = clGetPlatformInfo(id, CL_PLATFORM_EXTENSIONS, size, extensions, NULL);
		std::cout << "Platform extensions "; std::cout << extensions << std::endl;

		for (unsigned int j = 0; j < numDevices; j++) {

			std::cout << "DEVICE:" << j << "  [PLATFORM:" << i << "]" << std::endl;

			std::cout << "  Device ID"; std::cout << deviceIds[j] << std::endl;
			cl_device_id deviceId = deviceIds[j];

			cl_uint maxComputeUnits = 0;
			err = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &maxComputeUnits, NULL);

			std::cout << "  Max compute units "; std::cout << maxComputeUnits << std::endl;

			cl_uint maxWorkItemDimensions = 0;
			err = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &maxWorkItemDimensions, NULL);
			std::cout << ("  Max work item dimensions "); std::cout << maxWorkItemDimensions << " ";

			size_t* workItemSizes;
			workItemSizes = (size_t*)alloca(sizeof(size_t)*maxWorkItemDimensions);
			err = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_ITEM_SIZES, maxWorkItemDimensions*sizeof(size_t), workItemSizes, NULL);

			std::cout << std::endl;

			size_t maxWorkGroupSize = 0;
			err = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkGroupSize, NULL);

			std::cout << ("  Max work group size "); std::cout << maxWorkGroupSize << std::endl;

			cl_int deviceVendorId = 0;
			err = clGetDeviceInfo(deviceId, CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &deviceVendorId, NULL);
			std::cout << ("  Device vendor id "); std::cout << deviceVendorId << std::endl;

			cl_uint maxClockFrequency = 0;
			err = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &maxClockFrequency, NULL);

			std::cout << ("  Max clock frequency "); std::cout << maxClockFrequency << std::endl;

			cl_uint deviceAddressBits = 0;
			err = clGetDeviceInfo(deviceId, CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &deviceAddressBits, NULL);

			std::cout << ("  Device address bits "); std::cout << deviceAddressBits << std::endl;

			cl_ulong deviceMaxMemAllocSize = 0;
			err = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &deviceMaxMemAllocSize, NULL);

			std::cout << ("  Device max memory alloc size "); std::cout << deviceMaxMemAllocSize << std::endl;
		}
	}

	err = clGetPlatformIDs(0, NULL, &numPlatforms);

	std::cout << "Number of platforms: " << numPlatforms << std::endl;
	platformIds = (cl_platform_id*)alloca(sizeof(cl_platform_id)*numPlatforms);
	err = clGetPlatformIDs(numPlatforms, platformIds, NULL);
	if (err != CL_SUCCESS) {
		std::cout << "Error: Failed to get Platform IDs\n";
	}
}