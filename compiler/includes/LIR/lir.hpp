#ifndef LIR_LUBEX_H
#define LIR_LUBEX_H

#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include "../parser/ast.hpp"
#include "../evaluator.hpp"
#include "debug.hpp"

extern int lastId;

typedef std::string IRType;
typedef std::string IRName;

class LIRException : public std::exception {
    public:
        Error error;
        LIRException(Error err): error(err) {}
};

struct IRValue {
    IRName name;
    IRType type;
    virtual ~IRValue() = default;
    virtual void debug() const = 0;
    IRValue(const IRName& n, IRType t) : name(n), type(t) {}
};

struct IRMember : IRValue {
    IRMember(const IRName& name, IRType type, size_t idx): IRValue{name, type}, index(idx) {}
    size_t index;

    void debug() const override {
        DEBUG_OUTPUT << name << ": " << type << "\n";
    }
};

struct IRArg : IRValue {
    IRArg(const IRName& name, IRType type): IRValue{name, type} {}
    void debug() const override {
        DEBUG_OUTPUT << name << ": " << type << "\n";
    }
};

struct IRAlloca : IRValue {
    IRAlloca(const IRName& name, IRType type)
        : IRValue{name, type} {}

    void debug() const override {
        DEBUG_OUTPUT << name << ": " << type << " = alloca\n";
    }
};

struct IRAllocaStruct : IRValue {
    IRAllocaStruct(const IRName& name, IRType type): IRValue{name, type} {}

    void debug() const override {
        DEBUG_OUTPUT << name << ": " << type << " = alloca struct\n";
    }
};

struct IRStore : IRValue {
    IRValue* value;
    IRValue* ptr;

    IRStore(IRValue* ptr, IRValue* value)
        : IRValue{"%" + std::to_string(lastId++), "%tmp"}, value(value), ptr(ptr) {}

    void debug() const override {
        DEBUG_OUTPUT << "store " << value->name << " -> " << ptr->name << "\n";
    }
};

struct IRCall : IRValue {
    std::string funcName;
    std::vector<IRValue*> args;

    IRCall(const IRName& funcName, IRType type, std::vector<IRValue*> args)
        : IRValue{"%" + std::to_string(lastId++), type}, funcName(funcName), args(std::move(args)) {}

    void debug() const override {
        DEBUG_OUTPUT << name << " = call " << funcName << "(";
        for (size_t i = 0; i < args.size(); ++i) {
            DEBUG_OUTPUT << args[i]->name;
            if (i + 1 < args.size()) DEBUG_OUTPUT << ", ";
        }
        DEBUG_OUTPUT << "): " << type << "\n";
    }
};

struct IRFunction : IRValue {
    std::vector<IRValue*> args;
    std::vector<std::unique_ptr<IRValue>> body;
    IRType returnType;
    IRName className;
    bool isStatic;

    IRFunction(const IRName& funcName, std::vector<IRValue*> args, IRType returnType, IRName className, bool isStatic)
        : IRValue{funcName, returnType}, args(args), returnType(returnType), className(className), isStatic(isStatic) {}

    void debug() const override {
        DEBUG_OUTPUT << "function " << name << "(";
        for (size_t i = 0; i < args.size(); ++i) {
            DEBUG_OUTPUT << args[i]->name << ": " << args[i]->type;
            if (i + 1 < args.size()) DEBUG_OUTPUT << ", ";
        }
        DEBUG_OUTPUT << ") -> " << returnType << "\n";
        for (auto& instr : body) {
            DEBUG_OUTPUT << "\t";
            instr->debug();
        }
        DEBUG_OUTPUT << "\n";
    }
};

struct IRNumber : IRValue {
    double number;

    IRNumber(const IRName& name, IRType type, double number)
        : IRValue{name, type}, number(number) {}

    void debug() const override {
        DEBUG_OUTPUT << name << ": " << type << " = " << number << "\n";
    }
};

struct IRVariableRead : IRValue {
    IRVariableRead(const IRName& name, IRType type) : IRValue{name, type} {}

    void debug() const override {
        DEBUG_OUTPUT << "load " << name << ": " << type << "\n";
    }
};

struct IRStruct : IRValue {
    std::vector<std::unique_ptr<IRValue>> data;
    IRStruct(const IRName& name, std::vector<std::unique_ptr<IRValue>> body) : IRValue{name, "%tmp"}, data(std::move(body)) {}

    void debug() const override {
        DEBUG_OUTPUT << "struct " << name << "\n";
        for (auto& val : data) {
            DEBUG_OUTPUT << "\t";
            val->debug();
        }
        DEBUG_OUTPUT << "\n";
    }
};

struct IRAccess : IRValue {
    IRValue* object;
    int memberName;

    IRAccess(const IRName& name, const IRType& type, IRValue* object, const std::string& memberName) : IRValue(name, type), object(object), memberName(std::stoi(memberName)) {}

    void debug() const override {
        DEBUG_OUTPUT << name + " = access " + object->name + ", " + std::to_string(memberName) + "\n";
    }
};

struct IRClass : IRValue {
    IRName name;

    IRClass(const IRName& name, const IRName& normalName) : IRValue(name, normalName) {}

    void debug() const override {
        DEBUG_OUTPUT << name << " ";
    }
};

struct IRReturn : IRValue {
    IRValue* value;
    IRReturn(const IRType& type, IRValue* value): IRValue("%tmp", type), value(value) {}

    void debug() const override {
        if (value) DEBUG_OUTPUT << "ret " << value->name << " : " << type << "\n";
        else DEBUG_OUTPUT << "ret void\n";
    }
};

struct IRString : IRValue {
    std::string value;

    IRString(const IRName& name, IRType type, std::string value)
        : IRValue{name, type}, value(std::move(value)) {}

    void debug() const override {
        DEBUG_OUTPUT << name << ": " << type << " = \"" << value << "\"\n";
    }
};

struct LIRGenerate {
    IRValue* mainValue;
    std::vector<std::unique_ptr<IRValue>> code;
};

std::vector<std::unique_ptr<IRValue>> generateLIR(std::vector<std::unique_ptr<ASTNode>>& nodes, Context* ctx);

#endif // LIR_LUBEX_H