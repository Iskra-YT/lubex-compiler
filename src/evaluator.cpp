#include "evaluator.hpp"
#include <iostream>

void Context::declare(std::unique_ptr<Symbol> sym) {
    if (symbols.contains(sym->name->value)) {
        errors.push_back(Error(sym->name->position, "Symbol '" + sym->name->value + "' is already defined"));
        return;
    }
    symbols[sym->name->value] = std::move(sym);
}

Symbol* Context::lookup(const IdentyfierNode* name) {
    if (symbols.contains(name->value)) {
        return symbols[name->value].get();
    } 

    if (parent) {
        return parent->lookup(name);
    }

    errors.emplace_back(name->position, "Undefined identifier '" + name->value + "'");
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

void printSymbol(const Symbol* sym, int level = 0) {
    if (!sym) return;
    printIndent(level);
    std::cout << "Symbol: " << (sym->name ? sym->name->value : "nullptr")
              << " [" << symbolKindToString(sym->kind) << "]"
              << ", mangled: " << sym->mangledName << "\n";
    if (sym->type && sym->kind != SymbolKind::CLASS) {
        printIndent(level + 1);
        std::cout << "Type:\n";
        printSymbol(sym->type, level + 2);
    }
}

void printContext(const Context* ctx, int level) {
    if (!ctx) return;
    printIndent(level);
    std::cout << "Context [" << passPhaseToString(ctx->phase) << "], Symbol: \n";
    if (ctx->generativeSymbol) printSymbol(ctx->generativeSymbol, level + 1);

    for (const auto& [name, sym] : ctx->symbols) {
        printSymbol(sym.get(), level + 2);
    }

    for (const auto& child : ctx->children) {
        printContext(child.get(), level + 2);
    }

    for (auto err : ctx->errors) {
        printIndent(level + 2);
        std::cout << "Error: " << err.returnError() << "\n"; 
    }
}