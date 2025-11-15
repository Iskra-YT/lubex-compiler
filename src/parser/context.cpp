#include "parser/context.hpp"
#include "parser/parser.hpp"

void Parser::initVarDecl() {
    varDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "let", [](Token&, void*){}, false, false},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){ 
            auto& c = *(VarDeclContext*)ctx;
            c.name = parseTerm();
        }, true, false},
        {TokenType::DELIMITER_TOKEN, ":", [](Token&, void*){}, false, false},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){ 
            auto& c = *(VarDeclContext*)ctx;
            c.type = parseTerm();
        }, true, false},
        {TokenType::ASSIGNMENT_TOKEN, "=", [](Token&, void*){}, false, true},
        {TokenType::ANY, "", [&](Token&, void* ctx){
            auto& c = *(VarDeclContext*)ctx;
            c.valueNode = parseExpr();
        }, true, true},
    };

    varDeclInstr.finalize = [](PositionSpan span, void* ctx){
        VarDeclContext* c = (VarDeclContext*)ctx;
        return std::make_unique<VariableDeclarationNode>(span, std::move(c->name), std::move(c->valueNode), std::move(c->type));
    };
}

void Parser::initFuncDecl() {
    funcDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "func", [](Token&, void*){}, false, false},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token&, void* ctx){
            auto& c = *(FuncDeclContext*)ctx;
            c.name = parseTerm();
        }, true, false},
        {TokenType::DELIMITER_TOKEN, "(", [&](Token& t, void* ctx){
            auto& c = *(FuncDeclContext*)ctx;
            c.parameters = parseFunctionArgs();
        }, true, false},
        {TokenType::DELIMITER_TOKEN, ")", [](Token&, void*){}, false, false},
        {TokenType::DELIMITER_TOKEN, ":", [](Token&, void*){}, false, false},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){
            auto& c = *(FuncDeclContext*)ctx;
            c.returnType = parseTerm();
        }, true, false},
        {TokenType::DELIMITER_TOKEN, "->", [](Token&, void*){}, false, true},
        {TokenType::DELIMITER_TOKEN, "{", [&](Token&, void* ctx){
            auto& c = *(FuncDeclContext*)ctx;
            c.body = parseBlock();
        }, true, true},
        {TokenType::DELIMITER_TOKEN, "}", [](Token&, void*){}, false, true}
    };

    funcDeclInstr.finalize = [](PositionSpan span, void* ctx){
        FuncDeclContext* c = (FuncDeclContext*)ctx;
        return std::make_unique<FunctionDeclaration>(span, std::move(c->name), std::move(c->returnType), std::move(c->parameters), std::move(c->body));
    };
}