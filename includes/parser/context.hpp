#ifndef CONTEXT_LUBEX_HPP
#define CONTEXT_LUBEX_HPP

#include "parser/parser.hpp"

struct VarDeclContext {
    std::string name;
    std::unique_ptr<ASTNode> type;
    std::unique_ptr<ASTNode> valueNode;
};

#endif // CONTEXT_LUBEX_HPP