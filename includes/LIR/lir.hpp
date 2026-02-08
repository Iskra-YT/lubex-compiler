#ifndef LIR_LUBEX_H
#define LIR_LUBEX_H

#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include "../parser/ast.hpp"
#include "../evaluator.hpp"

extern int lastId;

typedef std::string IRType;
typedef std::string IRName;

struct IRValue {
    IRName name;
    IRType type;
    virtual ~IRValue() = default;
    virtual void debug() const = 0;
    IRValue(const IRName& n, IRType t) : name(n), type(t) {}
};

struct IRArg : IRValue {
    IRArg(const IRName& name, IRType type): IRValue{name, type} {}
    void debug() const override {
        std::cout << name << ": " << type << "\n";
    }
};

struct IRAlloca : IRValue {
    IRAlloca(const IRName& name, IRType type)
        : IRValue{name, type} {}

    void debug() const override {
        std::cout << name << ": " << type << " = alloca\n";
    }
};

struct IRStore : IRValue {
    IRValue* value;
    IRValue* ptr;

    IRStore(IRValue* ptr, IRValue* value)
        : IRValue{"%" + std::to_string(lastId++), "%tmp"}, value(value), ptr(ptr) {}

    void debug() const override {
        std::cout << "store " << value->name << " -> " << ptr->name << "\n";
    }
};

struct IRCall : IRValue {
    std::string funcName;
    std::vector<IRValue*> args;

    IRCall(const IRName& funcName, IRType type, std::vector<IRValue*> args)
        : IRValue{"%" + std::to_string(lastId++), type}, funcName(funcName), args(std::move(args)) {}

    void debug() const override {
        std::cout << name << " = call " << funcName << "(";
        for (size_t i = 0; i < args.size(); ++i) {
            std::cout << args[i]->name;
            if (i + 1 < args.size()) std::cout << ", ";
        }
        std::cout << "): " << type << "\n";
    }
};

struct IRFunction : IRValue {
    std::string funcName;
    std::vector<IRValue*> args;
    std::vector<std::unique_ptr<IRValue>> body;
    IRType returnType;

    IRFunction(const IRName& funcName, std::vector<IRValue*> args, IRType returnType)
        : IRValue{"%tmp", returnType}, funcName(funcName), args(args), returnType(returnType) {}

    void debug() const override {
        std::cout << "function " << funcName << "(";
        for (size_t i = 0; i < args.size(); ++i) {
            std::cout << args[i]->name << ": " << args[i]->type;
            if (i + 1 < args.size()) std::cout << ", ";
        }
        std::cout << ") -> " << returnType << "\n";
        for (auto& instr : body) {
            std::cout << "\t";
            instr->debug();
        }
    }
};

struct IRNumber : IRValue {
    double number;

    IRNumber(const IRName& name, IRType type, double number)
        : IRValue{name, type}, number(number) {}

    void debug() const override {
        std::cout << name << ": " << type << " = " << number << "\n";
    }
};

struct IRVariableRead : IRValue {
    IRVariableRead(const IRName& name, IRType type) : IRValue{name, type} {}

    void debug() const override {
        std::cout << "load " << name << ": " << type << "\n";
    }
};

struct LIRGenerate {
    IRValue* mainValue;
    std::vector<std::unique_ptr<IRValue>> code;
};

std::vector<std::unique_ptr<IRValue>> generateLIR(std::vector<std::unique_ptr<ASTNode>> nodes, Context& ctx);

#endif // LIR_LUBEX_H