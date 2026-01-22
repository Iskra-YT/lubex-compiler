#ifndef EVALUATOR_LUBEX_HPP
#define EVALUATOR_LUBEX_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "error.hpp"
#include "parser/ast.hpp"

struct IdentyfierNode;

enum class SymbolKind {
    NOT,
    VARIABLE,
    FUNCTION,
    CLASS,
    MODULE
};

enum class PassPhase {
    DECLARATION,
    MIDPASS,
    TYPE_CHECK
};

struct Symbol {
    SymbolKind kind;
    IdentyfierNode* name;
    Symbol* type;
    Context* scope = nullptr;

    Symbol(SymbolKind kind, IdentyfierNode* name, Symbol* type) : kind(kind), name(name), type(type) {}
};

struct Context {
    Context* parent = nullptr;
    SymbolKind symbolKind;
    std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols;
    std::vector<std::unique_ptr<Context>> children;
    std::vector<Error> errors;
    
    PassPhase phase;
    Context(Context* parent = nullptr) : parent(parent) {}

    void declare(std::unique_ptr<Symbol> sym);
    Symbol* lookup(const IdentyfierNode* name);
    Context* addChild();
    std::vector<Error> getErrors();
};

#endif // EVALUATOR_LUBEX_HPP