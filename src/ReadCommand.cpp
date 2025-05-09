#include "ReadCommand.h"
#include <stdexcept>
#include <string>

void ReadCommand::run(FunctionCalculator& calc, std::istream& args)
{
    std::string filePath;
    args >> filePath;

    if (filePath.empty())
        throw std::invalid_argument("No file path provided.");

    calc.executeFromFile(filePath);
}
