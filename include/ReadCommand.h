#pragma once

#include "FunctionCalculator.h"
#include <string>
#include <istream>

class ReadCommand
{
public:
    static void run(FunctionCalculator& calc, std::istream& args);
};
