#include "stdafx.h"
#include "InputParser.h"

// Standard Values for measureOn and 250 generations to process
int measureOn = 0;
int generations = 250;
int numThreads = 8;
int plID = 0;
int dvID = 0;

std::string inputFile;
std::string outputFile;
std::string Mode;
std::string sel_device;

cl_device_type device_type;

// shows how to use command inputs
void show_usage(char *argv)
{
	std::cerr << "Usage: " << argv << " <option(s)> "
		<< "Options:\n"
		<< "\t--help\t\tShow this help message\n"
		<< "\t--load \t\tSpecify the input .gol file\n"
		<< "\t--save \t\tSpecify the output .gol file\n"
		<< "\t--generations \tSpecify the generations (Standard: 250)\n"
		<< "\t--measure \tSpecify if you would like to measure performance"
		<< std::endl;
}

// check command line inputs and set global values 
int checkInputs(int argc, char *argv[])
{
	if (argc < 3)
	{
		show_usage(argv[0]);
		return 0;
	}

	// Input Handling
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--help") {
			show_usage(argv[0]);
			return 0;
		}
		else if (arg == "--load") {
			if (i + 1 < argc){
				inputFile = argv[i + 1];
			}
			else{
				return 0;
			}
		}
		else if (arg == "--save") {
			if (i + 1 < argc){
				outputFile = argv[i + 1];
			}
			else{
				return 0;
			}
		}
		else if (arg == "--generations") {
			if (i + 1 < argc){
				generations = atoi(argv[i + 1]);
			}
			else{
				return 0;
			}
		}
		else if (arg == "--measure") {
			measureOn = 1;
		}
		else if (arg == "--mode") {
			if (i + 1 < argc) {
				Mode = argv[i + 1];
			}
			else {
				return 0;
			}
		}
		else if (arg == "--threads") {
			if (i + 1 < argc) {
				numThreads = atoi(argv[i + 1]);
			}
			else {
				return 0;
			}
		}
		else if (arg == "--device") {
			if (i + 1 < argc) {
				sel_device = argv[i + 1];
				if (sel_device == "cpu") {
					device_type = CL_DEVICE_TYPE_CPU;
				}
				else if (sel_device == "gpu") {
					device_type = CL_DEVICE_TYPE_GPU;
				}
				else {
					return 0;
				}
			}
			else {
				return 0;
			}
		}

		else if (arg == "--platformId") {
			if (i + 1 < argc) {
				plID = atoi(argv[i + 1]);
			}
			else {
				return 0;
			}
		}
		else if (arg == "--deviceId") {
			if (i + 1 < argc) {
				dvID = atoi(argv[i + 1]);
			}
			else {
				return 0;
			}
		}

	}
	return 1;
}