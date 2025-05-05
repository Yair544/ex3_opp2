#pragma once

#include <vector>
#include <memory>
#include <string>
#include <iosfwd>
#include <optional>
#include <iostream>


class Operation;


class FunctionCalculator
{
public:
    FunctionCalculator(std::istream& istr, std::ostream& ostr);
    void run();

private:
    void eval();
    void del();
    void help();
    void exit();

    template <typename FuncType>
    void binaryFunc()
    {
        auto f0 = readOperationIndex();
        auto f1 = readOperationIndex();

        if (!f0 || !f1)
            throw std::invalid_argument("Invalid arguments: operation does not exist in the operation list.");
        m_operations.push_back(std::make_shared<FuncType>(m_operations[*f0], m_operations[*f1]));
    }

    template <typename FuncType>
    void unaryFunc()
    {
        auto idx = readOperationIndex();
        if (!idx)
            throw std::invalid_argument("Invalid arguments: operation does not exist in the operation list.");
        m_operations.push_back(std::make_shared<FuncType>(m_operations[*idx]));
    }

    template <typename FuncType>
    void unaryWithIntFunc()
    {
        int value = 0;
        m_istr >> value;

        if (!m_istr)
            throw std::invalid_argument("Invalid arguments: operation does not exist in the operation list.");
        m_operations.push_back(std::make_shared<FuncType>(value));
    }
    void printOperations() const;

    enum class Action
    {
        Invalid,
        Eval,
        Iden,
        Tran,
        Scal,
        Sub,
        Add,
        Mul,
        Comp,
        Del,
        Help,
        Exit,
    };

    struct ActionDetails
    {
        std::string command;
        std::string description;
        Action action;
    };

    using ActionMap = std::vector<ActionDetails>;
    using OperationList = std::vector<std::shared_ptr<Operation>>;

    const ActionMap m_actions;
    OperationList m_operations;
    bool m_running = true;
    std::istream& m_istr;
    std::ostream& m_ostr;

    std::optional<int> readOperationIndex() const;
    Action readAction() const;

    void runAction(Action action);

    ActionMap createActions() const;
    OperationList createOperations() const ;
};
