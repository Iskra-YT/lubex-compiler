#ifndef EVALUATOR_LUBEX_HPP
#define EVALUATOR_LUBEX_HPP

#include <vector>
#include <unordered_map>
#include <iostream>
#include <string>
#include <memory>
#include "error.hpp"
#include "parser/ast.hpp"
#include "debug.hpp"

struct IdentyfierNode;

enum class SymbolKind {
    NOT,
    VARIABLE,
    FUNCTION,
    CLASS,
    MODULE,
    ERROR
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
    ASTNode* node;
    std::string mangledName;
    size_t classMemberIndex;
    bool isStatic = false;
    std::string forcedMangle = "";
    std::vector<Symbol*> classTypes;

    Symbol(SymbolKind kind, IdentyfierNode* name, Symbol* type, ASTNode* node) : kind(kind), name(name), type(type), node(node) {}
    Symbol* clone() const {
        Symbol* copy = new Symbol(kind, name, type, node);
        copy->mangledName = mangledName;
        copy->classMemberIndex = classMemberIndex;
        copy->isStatic = isStatic;
        copy->forcedMangle = forcedMangle;
        copy->scope = scope;
        return copy;
    }

    Symbol* createInstance() {
        auto sym = clone();

        sym->kind = SymbolKind::VARIABLE;
        sym->type = this;

        return sym;
    }
};

struct Context {
    static int nextId;
    int debugId;

    Context* parent = nullptr;
    SymbolKind symbolKind;
    std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols;
    std::vector<std::unique_ptr<Context>> children;
    std::vector<Error> errors;
    
    PassPhase phase;
    Symbol* generativeSymbol = nullptr;
    Context(Context* parent_) : parent(parent_), debugId(nextId++) {
        DEBUG_OUTPUT << "[Context CREATED] id=" << debugId << " parent=" << (parent ? std::to_string(parent->debugId) : "nullptr") << "\n";
    }

    ~Context() {
        DEBUG_OUTPUT << "[Context DESTROYED] id=" << debugId << " parent=" << (parent ? std::to_string(parent->debugId) : "nullptr") << "\n";
    }

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = default;
    Context& operator=(Context&&) = default;

    void declare(std::unique_ptr<Symbol> sym);
    Symbol* lookup(const IdentyfierNode* name, bool getError = true);
    Symbol* lookup(const std::string name, PositionSpan span, bool getError = true);
    Symbol* lookup(const std::string name);
    Context* addChild();
    std::vector<Error> getErrors();
};

void normalizeSymbols(Context& ctx, const std::string& prefix = "");
void printContext(const Context* ctx, const std::string& prefix = "", bool isLast = true);

#endif // EVALUATOR_LUBEX_HPP