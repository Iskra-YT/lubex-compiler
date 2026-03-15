#include "evaluator.hpp"
#include <iostream>

int Context::nextId = 0;

void Context::declare(std::unique_ptr<Symbol> sym) {
    if (lookup(sym->name, false)) {
        errors.push_back(Error(sym->name->position, "Symbol '" + sym->name->value + "' is already defined"));
        return;
    }
    symbols[sym->name->value] = std::move(sym);
}

Symbol* Context::lookup(const IdentyfierNode* name, bool getError) {
    if (symbols.contains(name->value)) {
        return symbols[name->value].get();
    } 

    if (parent) {
        return parent->lookup(name, getError);
    }

    if (getError) {
        errors.emplace_back(name->position, "Undefined identifier '" + name->value + "'");
    }

    return nullptr;
}

Symbol* Context::lookup(const std::string name, PositionSpan span, bool getError) {
    if (symbols.contains(name)) {
        return symbols[name].get();
    } 

    if (parent) {
        return parent->lookup(name, span, getError);
    }

    if (getError) {
        errors.emplace_back(span, "Undefined identifier '" + name + "'");
    }

    return nullptr;
}

Symbol* Context::lookup(const std::string name) {
    if (symbols.contains(name)) {
        return symbols[name].get();
    } 

    if (parent) {
        return parent->lookup(name);
    }

    return nullptr;
}

Context* Context::addChild() {
    children.push_back(std::make_unique<Context>(this));
    return children.back().get();
}

std::vector<Error> Context::getErrors() {
    std::vector<Error> allErrors = errors;

    for (const auto& child : children) {
        std::vector<Error> childErrors = child->getErrors();
        allErrors.insert(allErrors.end(), childErrors.begin(), childErrors.end());
    }

    return allErrors;
}

void normalizeSymbols(Context& ctx, const std::string& prefix) {
    for (auto& [name, sym] : ctx.symbols) {
        std::string mangledName = prefix.empty() ? name : prefix + "." + name;
        sym->mangledName = mangledName;
        if (sym->scope) {
            normalizeSymbols(*sym->scope, mangledName);
        }
    }
}

void printIndent(int level) {
    for (int i = 0; i < level; ++i) std::cout << "  ";
}

std::string symbolKindToString(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::NOT: return "NOT";
        case SymbolKind::VARIABLE: return "VARIABLE";
        case SymbolKind::FUNCTION: return "FUNCTION";
        case SymbolKind::CLASS: return "CLASS";
        case SymbolKind::MODULE: return "MODULE";
        case SymbolKind::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string passPhaseToString(PassPhase phase) {
    switch (phase) {
        case PassPhase::DECLARATION: return "DECLARATION";
        case PassPhase::MIDPASS: return "MIDPASS";
        case PassPhase::TYPE_CHECK: return "TYPE_CHECK";
        default: return "UNKNOWN";
    }
}

void printSymbol(const Symbol* sym, const std::string& prefix = "", bool isLast = true) {
    if (!sym) return;

    std::cout << prefix;
    std::cout << (isLast ? "└─ " : "├─ ");
    std::cout << "Symbol: " << (sym->name ? sym->name->value : "nullptr")
              << " [" << symbolKindToString(sym->kind) << "]"
              << ", mangled: " << sym->mangledName << "\n";

    if (sym->type && sym->kind != SymbolKind::CLASS) {
        printSymbol(sym->type, prefix + (isLast ? "   " : "│  "), true);
    }
}

void printContext(const Context* ctx, const std::string& prefix, bool isLast, std::unordered_set<const Context*>& visited) {
    if (!ctx) return;

    if (visited.find(ctx) != visited.end()) {
        std::string genName = (ctx->generativeSymbol && ctx->generativeSymbol->name)
                              ? ctx->generativeSymbol->name->value
                              : "unnamed";
        std::cout << prefix << (isLast ? "└─ " : "├─ ")
                  << "[CYCLE DETECTED: generativeSymbol=\"" << genName << "\"]\n";
        return;
    }
    visited.insert(ctx);

    std::cout << prefix;
    std::cout << (isLast ? "└─ " : "├─ ");
    std::cout << "Context [" << passPhaseToString(ctx->phase) << "]\n";

    std::string childPrefix = prefix + (isLast ? "   " : "│  ");

    std::cout << childPrefix << "├─ GenerativeSymbol:\n";
    if (ctx->generativeSymbol)
        printSymbol(ctx->generativeSymbol, childPrefix + "│  ", true);

    std::cout << childPrefix << "├─ SymbolKind: " << symbolKindToString(ctx->symbolKind) << "\n";

    int count = 0;
    int totalSymbols = ctx->symbols.size();
    for (const auto& [name, sym] : ctx->symbols) {
        ++count;
        bool lastSym = (count == totalSymbols) && ctx->children.empty() && ctx->errors.empty();
        printSymbol(sym.get(), childPrefix, lastSym);
    }

    count = 0;
    int totalChildren = ctx->children.size();
    for (const auto& child : ctx->children) {
        ++count;
        bool lastChild = (count == totalChildren) && ctx->errors.empty();
        printContext(child.get(), childPrefix, lastChild, visited);
    }

    count = 0;
    int totalErrors = ctx->errors.size();
    for (auto err : ctx->errors) {
        ++count;
        bool lastErr = (count == totalErrors);
        std::cout << childPrefix << (lastErr ? "└─ " : "├─ ");
        std::cout << "Error: " << err.returnError() << "\n";
    }
}

void printContext(const Context* ctx, const std::string& prefix, bool isLast) {
    std::unordered_set<const Context*> visited;
    printContext(ctx, prefix, isLast, visited);
}