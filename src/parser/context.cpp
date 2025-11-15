#include "parser/context.hpp"
#include "parser/parser.hpp"

void Parser::initVarDecl() {
    varDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "let", [](Token&, void*){}, false},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){ 
            auto& c = *(VarDeclContext*)ctx;
            c.name = parseTerm();
        }, true},
        {TokenType::DELIMITER_TOKEN, ":", [](Token&, void*){}, false},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){ 
            auto& c = *(VarDeclContext*)ctx;
            c.type = parseTerm();
        }, true},
        {TokenType::ASSIGNMENT_TOKEN, "=", [](Token&, void*){}, false},
        {TokenType::ANY, "", [&](Token&, void* ctx){
            auto& c = *(VarDeclContext*)ctx;
            c.valueNode = parseExpr();
        }, true},
    };

    varDeclInstr.finalize = [](PositionSpan span, void* ctx){
        VarDeclContext* c = (VarDeclContext*)ctx;
        return std::make_unique<VariableDeclarationNode>(span, std::move(c->name), std::move(c->valueNode), std::move(c->type));
    };
}