#include "FunctionCalculator.h"

#include <string>
#include <iostream>


int main()
{
    try {
        FunctionCalculator(std::cin, std::cout).run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown fatal error occurred." << std::endl;
        return 1;
    }

    return 0;
}
