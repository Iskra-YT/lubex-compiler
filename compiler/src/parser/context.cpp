#include "parser/context.hpp"
#include "parser/parser.hpp"

void Parser::initVarDecl() {
    varDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "let", [](Token&, void*){}, true, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){ 
            auto& c = *(VarDeclContext*)ctx;
            c.name = parsePrimary();
        }, false, 0},
        {TokenType::DELIMITER_TOKEN, ":", [](Token&, void*){}, true, 2},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){ 
            auto& c = *(VarDeclContext*)ctx;
            c.type = parsePrimary();
            if (getCurrent().match(Token("?", TokenType::DELIMITER_TOKEN))) {
                advance();
                c.type = std::make_unique<NullableTypeNode>(
                    PositionSpan(c.type->position.start, getCurrent().position.end),
                    std::move(c.type)
                );
            }
        }, false, 2},
        {TokenType::ASSIGNMENT_TOKEN, "=", [](Token&, void*){}, true, 1},
        {TokenType::ANY, "", [&](Token&, void* ctx){
            auto& c = *(VarDeclContext*)ctx;
            Token tok = getCurrent();
            if (!tok.match(Token(";", TokenType::DELIMITER_TOKEN)) && tok.type != TokenType::EOF_TOKEN) {
                c.valueNode = parseExpr();
            } else {
                c.valueNode = nullptr;
            }
        }, false, 1},
    };

    varDeclInstr.finalize = [](PositionSpan span, void* ctx){
        VarDeclContext* c = (VarDeclContext*)ctx;
        if (c->valueNode) {
            return std::make_unique<VariableDeclarationNode>(
                span, std::move(c->name), std::move(c->valueNode), std::move(c->type), false
            );
        } else {
            return std::make_unique<VariableDeclarationNode>(
                span, std::move(c->name), nullptr, std::move(c->type), false
            );
        }
    };
}

void Parser::initFuncDecl() {
    funcDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "func", [](Token&, void*){}, true, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token&, void* ctx){
            allowCallAndMember = false;
            auto& c = *(FuncDeclContext*)ctx;
            c.name = parsePrimary();
            allowCallAndMember = true;
        }, false, 0},
        {TokenType::DELIMITER_TOKEN, "(", [&](Token& t, void* ctx){
            auto& c = *(FuncDeclContext*)ctx;
            c.parameters = parseFunctionArgs();
        }, false, 0},
        {TokenType::DELIMITER_TOKEN, ")", [](Token&, void*){}, true, 0},
        {TokenType::DELIMITER_TOKEN, ":", [](Token&, void*){}, true, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){
            auto& c = *(FuncDeclContext*)ctx;
            c.returnType = parseFactor();
            if (getCurrent().match(Token("?", TokenType::DELIMITER_TOKEN))) {
                advance();
                c.returnType = std::make_unique<NullableTypeNode>(
                    PositionSpan(c.returnType->position.start, getCurrent().position.end),
                    std::move(c.returnType)
                );
            }
        }, false, 0},
        {TokenType::DELIMITER_TOKEN, "{", [&](Token&, void* ctx){
            auto& c = *(FuncDeclContext*)ctx;
            c.body = parseBlock();
            c.isForward = false;
        }, false, 1},
        {TokenType::DELIMITER_TOKEN, "}", [](Token&, void*){}, true, 1}
    };

    funcDeclInstr.finalize = [](PositionSpan span, void* ctx){
        FuncDeclContext* c = (FuncDeclContext*)ctx;
        return std::make_unique<FunctionDeclaration>(span, std::move(c->name), std::move(c->returnType), std::move(c->parameters), std::move(c->body), c->isForward, c->isStatic, c->visibility, c->isOverride);
    };
}

void Parser::initClassDecl() {
    classDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "class", [](Token&, void*){}, true, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){
            allowCallAndMember = false;
            auto& c = *(ClassDeclContext*)ctx;
            c.name = parsePrimary();
            allowCallAndMember = true;
        }, false, 0},
        {TokenType::KEYWORD_TOKEN, "extends", [&](Token&, void*) {}, true, 2},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx) {
            allowCallAndMember = false;
            auto& c = *(ClassDeclContext*)ctx;
            c.extender = parsePrimary();
            allowCallAndMember = true;
        }, false, 2},
        {TokenType::DELIMITER_TOKEN, "{", [&](Token&, void* ctx){
            auto& c = *(ClassDeclContext*)ctx;
            c.members = parseBlock();
            c.isForward = false;
        }, false, 1},
        {TokenType::DELIMITER_TOKEN, "}", [](Token&, void*){}, true, 1}
    };

    classDeclInstr.finalize = [](PositionSpan span, void* ctx){
        ClassDeclContext* c = (ClassDeclContext*)ctx;
        return std::make_unique<ClassDeclNode>(span, std::move(c->name), std::move(c->members), c->isForward, c->visibility, std::move(c->extender));
    };
}

void Parser::initModuleDecl() {
    moduleDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "module", [](Token&, void*){}, true, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){
            allowCallAndMember = false;
            auto& c = *(ModuleDeclContext*)ctx;
            c.name = parsePrimary();
            allowCallAndMember = true;
        }, false, 0},
    };

    moduleDeclInstr.finalize = [](PositionSpan span, void* ctx){
        ModuleDeclContext* c = (ModuleDeclContext*)ctx;
        return std::make_unique<ModuleDeclaration>(span, std::move(c->name));
    };
}

void Parser::initConstDecl() {
    constDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "const", [](Token&, void*){}, true, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){ 
            auto& c = *(VarDeclContext*)ctx;
            c.name = parsePrimary();
        }, false, 0},
        {TokenType::DELIMITER_TOKEN, ":", [](Token&, void*){}, true, 2},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx){ 
            auto& c = *(VarDeclContext*)ctx;
            c.type = parsePrimary();
            if (getCurrent().match(Token("?", TokenType::DELIMITER_TOKEN))) {
                advance();
                c.type = std::make_unique<NullableTypeNode>(
                    PositionSpan(c.type->position.start, getCurrent().position.end),
                    std::move(c.type)
                );
            }
        }, false, 2},
        {TokenType::ASSIGNMENT_TOKEN, "=", [](Token&, void*){}, true, 1},
        {TokenType::ANY, "", [&](Token&, void* ctx){
            auto& c = *(VarDeclContext*)ctx;
            Token tok = getCurrent();
            if (!tok.match(Token(";", TokenType::DELIMITER_TOKEN)) && tok.type != TokenType::EOF_TOKEN) {
                c.valueNode = parseExpr();
            } else {
                c.valueNode = nullptr;
            }
        }, false, 1},
    };

    constDeclInstr.finalize = [](PositionSpan span, void* ctx){
        VarDeclContext* c = (VarDeclContext*)ctx;
        if (c->valueNode) {
            return std::make_unique<VariableDeclarationNode>(
                span, std::move(c->name), std::move(c->valueNode), std::move(c->type), true
            );
        } else {
            return std::make_unique<VariableDeclarationNode>(
                span, std::move(c->name), nullptr, std::move(c->type), true
            );
        }
    };
}

void Parser::initReturnDecl() {
    returnDeclInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "return", [](Token&, void*){}, true, 0},
        {TokenType::ANY, "", [&](Token& t, void* ctx){
            auto& c = *(ReturnDeclContext*)ctx;
            if (!t.match(Token(";", TokenType::DELIMITER_TOKEN)) && t.type != TokenType::EOF_TOKEN) {
                c.value = parseExpr();
            } else {
                c.value = nullptr;
            }
        }, false, 1}
    };

    returnDeclInstr.finalize = [](PositionSpan span, void* ctx) {
        ReturnDeclContext* c = (ReturnDeclContext*)ctx;
        if (c->value.get()) {
            return std::make_unique<ReturnNode>(span, std::move(c->value));
        } else {
            return std::make_unique<ReturnNode>(span, nullptr);
        }
    };
}

void Parser::initAttributes() {
    attributesInstr.steps = {
        {TokenType::DELIMITER_TOKEN, "@", [](Token&, void*){}, true, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx) {
            AttributesContext* c = (AttributesContext*)ctx;
            c->name = parsePrimary();
        }, false, 0},
        {TokenType::DELIMITER_TOKEN, "(", [](Token&, void*){}, true, 1},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx) {
            AttributesContext* c = (AttributesContext*)ctx;
            do {
                c->args.push_back(std::move(parsePrimary()));
            } while (getCurrent().match(",", TokenType::DELIMITER_TOKEN) && !tokens.empty());
        }, true, 1},
        {TokenType::DELIMITER_TOKEN, ")", [](Token&, void*){}, true, 1},
        {TokenType::ANY, "", [&](Token& t, void* ctx){
            AttributesContext* c = (AttributesContext*)ctx;
            c->value = parseExpr();
        }, false, 0},
    };

    attributesInstr.finalize = [](PositionSpan span, void* ctx) {
        AttributesContext* c = (AttributesContext*)ctx;
        return std::make_unique<AttributesNode>(span, std::move(c->name), std::move(c->args), std::move(c->value));
    };
}

void Parser::initIncludeDecl() {
    includeInstr.steps = {
        {TokenType::KEYWORD_TOKEN, "import", [](Token&, void*){}, true, 0},
        {TokenType::IDENTYFIER_TOKEN, "", [&](Token& t, void* ctx) {
            ImportContext* c = (ImportContext*)ctx;
            c->value = parsePrimary();
        }, false, 0}
    };

    includeInstr.finalize = [](PositionSpan span, void* ctx) {
        ImportContext* c = (ImportContext*)ctx;
        return std::make_unique<ImportNode>(span, std::move(c->value));
    };
}