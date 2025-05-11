#include "FunctionCalculator.h"
#include "SquareMatrix.h"
#include "Add.h"
#include "Sub.h"
#include "Comp.h"
#include "Identity.h"
#include "Transpose.h"
#include "Scalar.h"
#include "ReadCommand.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

FunctionCalculator::FunctionCalculator(std::istream& istr, std::ostream& ostr)
    : m_actions(createActions()), m_operations(createOperations()), m_istr(istr), m_ostr(ostr)
{
}

void FunctionCalculator::run()
{
    std::string line;
    askMaxFunctions();
    std::getline(std::cin, line); //clean std::cin

    do {
        m_ostr << '\n';
        printOperations();
        m_ostr << "Enter command ('help' for the list of available commands): ";

        if (!std::getline(std::cin, line))
            break;

        try {
            executeSingleCommand(line);
         
        }
        catch (const std::invalid_argument& e) {
            m_ostr << "Error: " << e.what() << "\n";
         
        }

    } while (m_running);
}

void FunctionCalculator::askMaxFunctions()
{
    int n;
    do {
        m_ostr << "Enter max number of functions (2 - 100): ";
        m_istr >> n;

        if (!m_istr || n < 2 || n > 100)
        {
            m_ostr << "Invalid input. Please enter a number between 2 and 100.\n";
            m_istr.clear();
            m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else
        {
            m_maxFunctions = n;
            break;
        }
    } while (true);
}

void FunctionCalculator::eval()
{
    ensureSpace();

    if (auto index = readOperationIndex(); index)
    {
        const auto& operation = m_operations[*index];
        int inputCount = operation->inputCount();
        int size = 0;
        m_istr >> size;

        if (!m_istr)
            throw std::invalid_argument("Expected matrix size.");

        if (size <= 1 || size > MAX_MAT_SIZE)
            throw std::invalid_argument("Matrix size must be between 2 and " + std::to_string(MAX_MAT_SIZE));

        auto matrixVec = std::vector<Operation::T>();
        if (inputCount > 1)
            m_ostr << "\nPlease enter " << inputCount << " matrices:\n";

        for (int i = 0; i < inputCount; ++i)
        {
            auto input = Operation::T(size);
            m_ostr << "\nEnter a " << size << "x" << size << " matrix:\n";
            m_istr >> input;
            matrixVec.push_back(input);
        }

        m_ostr << "\n";
        operation->print(m_ostr, matrixVec);
        m_ostr << " = \n" << operation->compute(matrixVec);
    }
}

void FunctionCalculator::del()
{
    if (auto i = readOperationIndex(); i)
    {
        m_operations.erase(m_operations.begin() + *i);
    }
}

void FunctionCalculator::help()
{
    m_ostr << "The available commands are:\n";
    for (const auto& action : m_actions)
        m_ostr << "* " << action.command << action.description << '\n';
    m_ostr << '\n';
}

void FunctionCalculator::exit()
{
    m_ostr << "Goodbye!\n";
    m_running = false;
}

void FunctionCalculator::printOperations() const
{
    m_ostr << "List of available matrix operations (" << m_operations.size()
        << " / " << m_maxFunctions << " used):\n";

    for (decltype(m_operations.size()) i = 0; i < m_operations.size(); ++i)
    {
        m_ostr << i << ". ";
        m_operations[i]->print(m_ostr, true);
        m_ostr << '\n';
    }
    m_ostr << '\n';
}

void FunctionCalculator::ensureSpace() const
{
    if (m_operations.size() >= m_maxFunctions)
        throw std::invalid_argument("Function list is full (max: " + std::to_string(m_maxFunctions) + ")");
}

std::optional<int> FunctionCalculator::readOperationIndex() const
{
    int i = 0;
    m_istr >> i;
    if (i >= static_cast<int>(m_operations.size()))
    {
        m_ostr << "Operation #" << i << " doesn't exist\n";
        return {};
    }
    return i;
}

FunctionCalculator::Action FunctionCalculator::readAction() const
{
    std::string action;
    m_istr >> action;

    const auto i = std::ranges::find(m_actions, action, &ActionDetails::command);
    return i != m_actions.end() ? i->action : Action::Invalid;
}

void FunctionCalculator::runAction(Action action)
{
    switch (action)
    {
    case Action::Eval:         eval();                     break;
    case Action::Add:          binaryFunc<Add>();          break;
    case Action::Sub:          binaryFunc<Sub>();          break;
    case Action::Comp:         binaryFunc<Comp>();         break;
    case Action::Read:         ReadCommand::run(*this, m_istr); break;
    case Action::Del:          del();                      break;
    case Action::Help:         help();                     break;
    case Action::Exit:         exit();                     break;
    case Action::Scal:         unaryWithIntFunc<Scalar>(); break;
    case Action::Resize:       resizeOperations();          break;
    default:
        throw std::invalid_argument("Command not found\n");
    }
}

FunctionCalculator::ActionMap FunctionCalculator::createActions() const
{
    return {
        {"eval", "(uate) num n - compute the result of function #num on an n׳n matrix", Action::Eval},
        {"scal", "(ar) val - scalar multiplication", Action::Scal},
        {"add",  " num1 num2 - add two operations", Action::Add},
        {"sub",  " num1 num2 - subtract two operations", Action::Sub},
        {"comp", "(osite) num1 num2 - compose two operations", Action::Comp},
        {"read", " file_path - execute commands from file", Action::Read},
        {"del",  "(ete) num - delete operation #num", Action::Del},
        {"help", " - print command list", Action::Help},
        {"exit", " - exit program", Action::Exit},
        { "resize", " n – change the maximum number of stored functions (2‑100)", Action::Resize },
    };
}

FunctionCalculator::OperationList FunctionCalculator::createOperations() const
{
    return {
        std::make_shared<Identity>(),
        std::make_shared<Transpose>(),
    };
}

void FunctionCalculator::executeFromFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file)
        throw std::invalid_argument("Failed to open file: " + filePath);

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line))
    {
        ++lineNumber;
        try {
            executeSingleCommand(line);
        }
        catch (const std::exception& e)
        {
            m_ostr << "Error (in file, line " << lineNumber << "): " << e.what() << "\n";
            if (!askUserToContinue()) break;
        }
    }
}

bool FunctionCalculator::askUserToContinue()
{
    m_ostr << "Continue reading the file? (y/n): ";
    std::string choice;
    std::cin >> choice;
    return choice == "y" || choice == "Y";
}

//void FunctionCalculator::executeSingleCommand(const std::string& line)
//{
//    std::istringstream lineStream(line);
//    std::string command;
//    lineStream >> command;
//
//    const auto it = std::ranges::find(m_actions, command, &ActionDetails::command);
//    if (it == m_actions.end())
//        throw std::invalid_argument("Command not found");
//
//    std::vector<std::string> tokens;
//    std::string token;
//    while (lineStream >> token)
//        tokens.push_back(token);
//
//    // Argument validation
//    switch (it->action)
//    {
//    case Action::Add:
//    case Action::Sub:
//    case Action::Comp:
//        if (tokens.size() != 2)
//            throw std::invalid_argument("Command '" + command + "' expects exactly 2 arguments.");
//        break;
//    case Action::Scal:
//    case Action::Del:
//    case Action::Read:
//    case Action::Resize:
//        if (tokens.size() != 1)
//            throw std::invalid_argument("Command '" + command + "' expects exactly 1 argument.");
//        break;
//    case Action::Help:
//    case Action::Exit:
//        if (!tokens.empty())
//            throw std::invalid_argument("Command '" + command + "' does not take any arguments.");
//        break;
//    default:
//        break;
//    }
//
//    std::ostringstream cleaned;
//    for (const auto& s : tokens)
//        cleaned << s << ' ';
//    std::istringstream argsStream(cleaned.str());
//
//    if (it->action == Action::Resize)
//    {
//        std::streambuf* oldBuf = m_istr.rdbuf(argsStream.rdbuf());
//        this->resizeFunctions();
//        m_istr.rdbuf(oldBuf);
//        return;
//    }
//
//    FunctionCalculator temp(argsStream, m_ostr);
//    temp.m_operations = this->m_operations;
//    temp.m_actions = this->m_actions;
//    temp.m_maxFunctions = this->m_maxFunctions;
//
//    temp.runAction(it->action);
//    this->m_operations = temp.m_operations;
//}

void FunctionCalculator::executeSingleCommand(const std::string& line)
{
    std::istringstream lineStream(line);
    std::string command;
    lineStream >> command;

    const auto it = std::ranges::find(m_actions, command, &ActionDetails::command);
    if (it == m_actions.end())
        throw std::invalid_argument("Command not found");

    std::vector<std::string> tokens;
    std::string token;
    while (lineStream >> token)
        tokens.push_back(token);

    // Argument validation
    switch (it->action)
    {
    case Action::Add:
    case Action::Sub:
    case Action::Comp:
        if (tokens.size() != 2)
            throw std::invalid_argument("Command '" + command + "' expects exactly 2 arguments.");
        break;
    case Action::Scal:
    case Action::Del:
    case Action::Read:
    case Action::Resize:
        if (tokens.size() != 1)
            throw std::invalid_argument("Command '" + command + "' expects exactly 1 argument.");
        break;
    case Action::Help:
    case Action::Exit:
        if (!tokens.empty())
            throw std::invalid_argument("Command '" + command + "' does not take any arguments.");
        break;
    default:
        break;
    }

    std::ostringstream cleaned;
    for (const auto& s : tokens)
        cleaned << s << ' ';
    std::istringstream argsStream(cleaned.str());

    // פעולות רגילות
    FunctionCalculator temp(argsStream, m_ostr);
    temp.m_operations = this->m_operations;
    temp.m_actions = this->m_actions;
    temp.m_maxFunctions = this->m_maxFunctions;

    temp.runAction(it->action);
    this->m_operations = temp.m_operations;
    this->m_maxFunctions = temp.m_maxFunctions;

}



//void FunctionCalculator::resizeFunctions()
//{
//    int newMax = 0;
//    m_istr >> newMax;
//
//    if (!m_istr || newMax < 2 || newMax > 100)
//        throw std::invalid_argument("Resize value must be an integer between 2 and 100.");
//
//    if (newMax < static_cast<int>(m_operations.size()))
//    {
//        m_ostr << "Warning: current list holds " << m_operations.size()
//            << " functions. Resize to " << newMax
//            << " will delete operations [" << newMax << " .. " << m_operations.size() - 1 << "].\n";
//
//        m_ostr << "Continue? (y/n): ";
//        std::string choice;
//        std::cin >> choice;
//
//        if (choice != "y" && choice != "Y")
//        {
//            // ❗️נדרש חריג אמיתי כדי להפסיק את הפקודה ולהודיע על כשל
//            throw std::invalid_argument("Resize aborted: too many functions for requested limit.");
//        }
//
//        // אם המשתמש מאשר — מוחקים את הפונקציות העודפות
//        m_operations.erase(m_operations.begin() + newMax, m_operations.end());
//    }
//
//    m_maxFunctions = newMax;
//    m_ostr << "Max functions set to " << m_maxFunctions << ".\n";
//}

void FunctionCalculator::resizeOperations()
{
    size_t newSize;
    m_istr >> newSize;

    if (newSize < 2 || newSize > 100)
        throw std::invalid_argument("Resize value must be between 2 and 100");

    if (newSize < m_operations.size())
    {
        m_ostr << "Warning: currently " << m_operations.size()
            << " operations stored. Resizing to " << newSize
            << " will delete operations [" << newSize << " .. " << m_operations.size() - 1 << "].\n";

        m_ostr << "Continue? (y/n): ";
        std::string choice;
        std::cin >> choice;

        if (choice != "y" && choice != "Y")
            throw std::invalid_argument("Resize aborted by user.");

        // מחיקת הפקודות המיותרות
        m_operations.erase(m_operations.begin() + newSize, m_operations.end());
    }

    m_maxFunctions = newSize;
    m_ostr << "Max functions set to " << m_maxFunctions << ".\n";
}
