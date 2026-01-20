#include "evaluator.hpp"

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
