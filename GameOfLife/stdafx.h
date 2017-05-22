// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <windows.h>

// inlucde OpenMP
#include <omp.h>
// include OpenCL and also use deprecated functions as we target also OpenCL 1.2
//#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL\cl.h>
#include <CL\opencl.h>


// TODO: reference additional headers your program requires here
