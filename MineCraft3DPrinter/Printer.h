#pragma once

#define LOG_MSG(msg) std::cout<<"PROGRAM LOG :" <<msg<<"    " << std::endl;


#include <Windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include  "Math.h"
#include "FileLoader.h"
#include "MeshSlicer.h"
#include "LayerRasterizer.h"