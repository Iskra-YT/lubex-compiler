#include "parser/context.hpp"
#include "parser/parser.hpp"

void Parser::initVarDecl() {
    varDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "let", [](Token&, void*){}, false, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){ 
            auto& c = *(VarDeclContext*)ctx;
            c.name = parseTerm();
        }, true, 0},
        {TokenType::DELIMITER_TOKEN, ":", [](Token&, void*){}, false, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){ 
            auto& c = *(VarDeclContext*)ctx;
            c.type = parseTerm();
        }, true, 0},
        {TokenType::ASSIGNMENT_TOKEN, "=", [](Token&, void*){}, false, 1},
        {TokenType::ANY, "", [&](Token&, void* ctx){
            auto& c = *(VarDeclContext*)ctx;
            c.valueNode = parseExpr();
        }, true, 1},
    };

    varDeclInstr.finalize = [](PositionSpan span, void* ctx){
        VarDeclContext* c = (VarDeclContext*)ctx;
        return std::make_unique<VariableDeclarationNode>(span, std::move(c->name), std::move(c->valueNode), std::move(c->type));
    };
}

void Parser::initFuncDecl() {
    funcDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "func", [](Token&, void*){}, false, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token&, void* ctx){
            allowCallAndMember = false;
            auto& c = *(FuncDeclContext*)ctx;
            c.name = parseTerm();
            allowCallAndMember = true;
        }, true, 0},
        {TokenType::DELIMITER_TOKEN, "(", [&](Token& t, void* ctx){
            auto& c = *(FuncDeclContext*)ctx;
            c.parameters = parseFunctionArgs();
        }, true, 0},
        {TokenType::DELIMITER_TOKEN, ")", [](Token&, void*){}, false, 0},
        {TokenType::DELIMITER_TOKEN, ":", [](Token&, void*){}, false, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){
            auto& c = *(FuncDeclContext*)ctx;
            c.returnType = parseTerm();
        }, true, 0},
        {TokenType::DELIMITER_TOKEN, "->", [](Token&, void*){}, false, 1},
        {TokenType::DELIMITER_TOKEN, "{", [&](Token&, void* ctx){
            auto& c = *(FuncDeclContext*)ctx;
            c.body = parseBlock();
            c.isForward = false;
        }, true, 1},
        {TokenType::DELIMITER_TOKEN, "}", [](Token&, void*){}, false, 1}
    };

    funcDeclInstr.finalize = [](PositionSpan span, void* ctx){
        FuncDeclContext* c = (FuncDeclContext*)ctx;
        return std::make_unique<FunctionDeclaration>(span, std::move(c->name), std::move(c->returnType), std::move(c->parameters), std::move(c->body), c->isForward);
    };
}

void Parser::initClassDecl() {
    classDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "class", [](Token&, void*){}, false, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){
            allowCallAndMember = false;
            auto& c = *(ClassDeclContext*)ctx;
            c.name = parseTerm();
            allowCallAndMember = true;
        }, true, 0},
        {TokenType::DELIMITER_TOKEN, "->", [](Token&, void*){}, false, 1},
        {TokenType::DELIMITER_TOKEN, "{", [&](Token&, void* ctx){
            auto& c = *(ClassDeclContext*)ctx;
            c.members = parseBlock();
            c.isForward = false;
        }, true, 1},
        {TokenType::DELIMITER_TOKEN, "}", [](Token&, void*){}, false, 1}
    };

    classDeclInstr.finalize = [](PositionSpan span, void* ctx){
        ClassDeclContext* c = (ClassDeclContext*)ctx;
        return std::make_unique<ClassDeclNode>(span, std::move(c->name), std::move(c->members), c->isForward);
    };
}

void Parser::initModuleDecl() {
    moduleDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "module", [](Token&, void*){}, false, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){
            allowCallAndMember = false;
            auto& c = *(ModuleDeclContext*)ctx;
            c.name = parseTerm();
            allowCallAndMember = true;
        }, true, 0},
    };

    moduleDeclInstr.finalize = [](PositionSpan span, void* ctx){
        ModuleDeclContext* c = (ModuleDeclContext*)ctx;
        return std::make_unique<ModuleDeclaration>(span, std::move(c->name));
    };
}