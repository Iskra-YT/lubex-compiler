#ifndef CONTEXT_LUBEX_HPP
#define CONTEXT_LUBEX_HPP

#include "parser/parser.hpp"

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
    bool isForward = true;
    bool isStatic = false;
};

struct ClassDeclContext {
    std::unique_ptr<ASTNode> name;
    std::vector<std::unique_ptr<ASTNode>> members;
    bool isForward = true;
};

struct ModuleDeclContext {
    std::unique_ptr<ASTNode> name;
};

#endif // CONTEXT_LUBEX_HPP