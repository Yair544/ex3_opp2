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
    do {
        m_ostr << '\n';
        printOperations();
        m_ostr << "Enter command ('help' for the list of available commands): ";

        std::string line;
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


void FunctionCalculator::eval()
{
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
    {
        m_ostr << "* " << action.command << action.description << '\n';
    }
    m_ostr << '\n';
}


void FunctionCalculator::exit()
{
    m_ostr << "Goodbye!\n";
    m_running = false;
}


void FunctionCalculator::printOperations() const
{
    m_ostr << "List of available matrix operations:\n";
    for (decltype(m_operations.size()) i = 0; i < m_operations.size(); ++i)
    {
        m_ostr << i << ". ";
        m_operations[i]->print(m_ostr,true);
        m_ostr << '\n';
    }
    m_ostr << '\n';
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
    auto action = std::string();
    m_istr >> action;

    const auto i = std::ranges::find(m_actions, action, &ActionDetails::command);
    if (i != m_actions.end())
    {
        return i->action;
    }

    return Action::Invalid;
}


void FunctionCalculator::runAction(Action action)
{
    switch (action)
    {
        default:
            m_ostr << "Unknown enum entry used!\n";
            break;

        case Action::Invalid:
            throw std::invalid_argument("Command not found\n");
            break;

        case Action::Eval:         eval();                     break;
        case Action::Add:          binaryFunc<Add>();          break;
        case Action::Sub:          binaryFunc<Sub>();          break;
        case Action::Comp:         binaryFunc<Comp>();         break;
        case Action::Read:         ReadCommand::run(*this, m_istr); break;
        case Action::Del:          del();                      break;
        case Action::Help:         help();                     break;
        case Action::Exit:         exit();                     break;
        //case Action::Iden:          unaryFunc<Identity>();      break;
        //case Action::Tran:          unaryFunc<Transpose>();      break;
        case Action::Scal:          unaryWithIntFunc<Scalar>();      break;
    }
}


FunctionCalculator::ActionMap FunctionCalculator::createActions() const
{
    return ActionMap
    {
        {
            "eval",
            "(uate) num n - compute the result of function #num on an n׳n matrix "
			"(that will be prompted)",
            Action::Eval
        },
        {
            "scal",
            "(ar) val - creates an operation that multiplies the "
			"given matrix by scalar val",
            Action::Scal
        },
        {
            "add",
            " num1 num2 - creates an operation that is the addition of the result of operation #num1 "
			"and the result of operation #num2",
            Action::Add
        },
         {
            "sub",
            " num1 num2 - creates an operation that is the subtraction of the result of operation #num1 "
			"and the result of operation #num2",
            Action::Sub
        },
        {
            "comp",
            "(osite) num1 num2 - creates an operation that is the composition of operation #num1 "
			"and operation #num2",
            Action::Comp
        },
        {
            "read",
            " file_path - reads and executes commands from a file",
            Action::Read
        },
        {
            "del",
            "(ete) num - delete operation #num from the operation list",
            Action::Del
        },
        {
            "help",
            " - print this command list",
            Action::Help
        },
        {
            "exit",
            " - exit the program",
            Action::Exit
        }
    };
}


FunctionCalculator::OperationList FunctionCalculator::createOperations() const
{
    return OperationList
    {
        std::make_shared<Identity>(),
        std::make_shared<Transpose>(),
    };
}

////////////////////////////////////////////////////////
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
        if (tokens.size() != 1)
            throw std::invalid_argument("Command '" + command + "' expects exactly 1 argument.");
        break;

    case Action::Read:
        if (tokens.size() != 1)
            throw std::invalid_argument("Command 'read' expects a single file path.");
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

    FunctionCalculator temp(argsStream, m_ostr);
    temp.m_operations = this->m_operations;
    temp.m_actions = this->m_actions;

    temp.runAction(it->action);
    this->m_operations = temp.m_operations;
}

