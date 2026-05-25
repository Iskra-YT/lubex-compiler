#ifndef CONTEXT_LUBEX_HPP
#define CONTEXT_LUBEX_HPP

#include "parser/parser.hpp"
#include "evaluator.hpp"

struct VarDeclContext {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> type;
    std::unique_ptr<ASTNode> valueNode;
};

struct ArgDeclContext {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> type;
};

struct FuncDeclContext {
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> returnType;
    std::vector<std::unique_ptr<ASTNode>> parameters;
    std::vector<std::unique_ptr<ASTNode>> body;
    VisibilityKind visibility;
    bool isForward = true;
    bool isStatic = false;
    bool isOverride = false;
};

struct ClassDeclContext {
    std::unique_ptr<ASTNode> name;
    std::vector<std::unique_ptr<ASTNode>> members;
    VisibilityKind visibility;
    std::unique_ptr<ASTNode> extender;
    bool isForward = true;
};

struct ModuleDeclContext {
    std::unique_ptr<ASTNode> name;
};

struct ReturnDeclContext {
    std::unique_ptr<ASTNode> value;
};

struct AttributesContext {
    std::unique_ptr<ASTNode> name;
    std::vector<std::unique_ptr<ASTNode>> args;
    std::unique_ptr<ASTNode> value;
};

struct ImportContext {
    std::unique_ptr<ASTNode> value;
};

#endif // CONTEXT_LUBEX_HPP