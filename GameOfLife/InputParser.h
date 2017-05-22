#pragma once
#ifndef INPUTPARSER_H
#define INPUTPARSER_H

// shows how to use command inputs
static void show_usage(char *argv);
// check command line inputs and set global values 
int checkInputs(int argc, char *argv[]);

extern int measureOn;
extern int generations;
extern int numThreads;
extern int plID;
extern int dvID;
extern std::string inputFile;
extern std::string outputFile;
extern std::string Mode;
extern std::string sel_device;

extern cl_device_type device_type;

#endif