#include "parser/parser.hpp"
#include <exception>
#include <filesystem>
#include "parser/context.hpp"

extern std::filesystem::path mainSource;

struct ASTNode;

Parser::Parser(std::vector<Token> toks) : tokens(toks) {
    initVarDecl();
    initFuncDecl();
    initClassDecl();
    initModuleDecl();
    initConstDecl();
    initReturnDecl();
    initAttributes();
    initIncludeDecl();
}

Token Parser::getCurrent() {
    if (position < tokens.size()) return tokens[position];
    return Token("", TokenType::EOF_TOKEN);
}

Token Parser::advance() {
    if (position < tokens.size()) position++;
    return getCurrent();
}

void Parser::pushError(Error error) {
    while (position < tokens.size() && !getCurrent().match(Token(";", TokenType::DELIMITER_TOKEN))) {
        advance();
    }

    if (getCurrent().match(Token(";", TokenType::DELIMITER_TOKEN))) {
        advance();
    }

    errors.push_back(error);
}

std::vector<Error> Parser::getErrors() {
    return errors;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> nodes;
    while (position < tokens.size() && getCurrent().type != TokenType::EOF_TOKEN) {
        nodes.push_back(parseStatement());
    }

    return nodes;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parseFunctionArgs() {
    std::vector<std::unique_ptr<ASTNode>> args;

    if (!getCurrent().match(Token("(", TokenType::DELIMITER_TOKEN))) {
        pushError(Error(getCurrent().position, "Expected '(' at beginning of function arguments",
                        mainSource.filename().string()));
        return args;
    }
    advance();

    if (getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
        return args;
    }

    while (!getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
        Token argNameToken = getCurrent();
        if (argNameToken.type != TokenType::IDENTYFIER_TOKEN) {
            pushError(Error(argNameToken.position, "Expected argument name", mainSource.filename().string()));
            return args;
        }
        advance();

        if (!getCurrent().match(Token(":", TokenType::DELIMITER_TOKEN))) {
            pushError(Error(getCurrent().position, "Expected ':' after argument name", mainSource.filename().string()));
            return args;
        }
        advance();

        Token typeToken = getCurrent();
        if (typeToken.type != TokenType::IDENTYFIER_TOKEN) {
            pushError(Error(typeToken.position, "Expected type for argument", mainSource.filename().string()));
            return args;
        }
        advance();

        std::unique_ptr<ASTNode> typeNode = std::make_unique<IdentyfierNode>(typeToken.position, typeToken.value);
        if (getCurrent().match(Token("?", TokenType::DELIMITER_TOKEN))) {
            advance();
            typeNode = std::make_unique<NullableTypeNode>(
                PositionSpan(typeToken.position.start, getCurrent().position.end), std::move(typeNode));
        }

        args.push_back(std::make_unique<ArgDeclaration>(
            PositionSpan(argNameToken.position.start, typeNode->position.end),
            std::make_unique<IdentyfierNode>(argNameToken.position, argNameToken.value), std::move(typeNode)));

        if (getCurrent().match(Token(",", TokenType::DELIMITER_TOKEN))) {
            advance();
        } else if (!getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
            pushError(
                Error(getCurrent().position, "Expected ',' or ')' in argument list", mainSource.filename().string()));
            return args;
        }
    }

    return args;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parseBlock() {
    std::vector<std::unique_ptr<ASTNode>> statements;

    if (!getCurrent().match(Token("{", TokenType::DELIMITER_TOKEN))) {
        pushError(Error(getCurrent().position, "Expected '{' at beginning of block", mainSource.filename().string()));
        return statements;
    }
    advance();

    while (!getCurrent().match(Token("}", TokenType::DELIMITER_TOKEN))) {
        auto stmt = parseStatement();
        if (!stmt) {
            while (position < tokens.size() && !getCurrent().match(Token("}", TokenType::DELIMITER_TOKEN))) {
                advance();
            }
            break;
        }
        statements.push_back(std::move(stmt));
    }

    if (!getCurrent().match(Token("}", TokenType::DELIMITER_TOKEN))) {
        pushError(Error(getCurrent().position, "Expected '}' at end of block", mainSource.filename().string()));
    }

    return statements;
}

std::unique_ptr<ASTNode> Parser::parseInstruction(InstructionSet& instrSet, void* context) {
    Position start = getCurrent().position.start;

    std::function<std::unique_ptr<ASTNode>(size_t)> tryStep;
    tryStep = [&](size_t currentStep) -> std::unique_ptr<ASTNode> {
        if (currentStep >= instrSet.steps.size()) {
            return instrSet.finalize(PositionSpan(start, getCurrent().position.end), context);
        }

        auto& step = instrSet.steps[currentStep];
        Token tok = getCurrent();

        bool matchesType = (step.expectedType == TokenType::ANY || tok.type == step.expectedType);
        bool matchesValue = (step.expectedValue.empty() || tok.value == step.expectedValue);
        bool matched = matchesType && matchesValue;

        if (step.optional == 0) {
            if (!matched) {
                pushError(Error(tok.position, "Unexpected token: " + tok.value, mainSource.filename().string()));
                return nullptr;
            }

            step.action(tok, context);
            if (step.consumesToken) advance();

            return tryStep(currentStep + 1);
        }

        if (matched) {
            Token saveTok = tok;
            auto saveState = this->position;
            step.action(tok, context);
            if (step.consumesToken) advance();
            auto taken = tryStep(currentStep + 1);
            if (taken) return taken;

            this->position = saveState;
            return tryStep(currentStep + 1);
        } else {
            return tryStep(currentStep + 1);
        }
    };

    return tryStep(0);
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    setDefaultVisibility();
    setDefaultOverride();

    auto node = parseExpr();
    if (!node) {
        pushError(Error(getCurrent().position, "Invalid expression in statement", mainSource.filename().string()));
        return nullptr;
    }

    if (!getCurrent().match(Token(";", TokenType::DELIMITER_TOKEN))) {
        pushError(Error(PositionSpan(getCurrent().position.start, getCurrent().position.end),
                        "Expected ';' after statement", mainSource.filename().string()));
        return nullptr;
    }
    advance();
    return std::make_unique<StatementNode>(PositionSpan(node->position.start, getCurrent().position.end),
                                           std::move(node));
}

std::unique_ptr<ASTNode> Parser::parseExpr() {
    Token tok = getCurrent();

    if (tok.match(Token("let", TokenType::KEYWORD_TOKEN))) {
        VarDeclContext ctx;
        return parseInstruction(varDeclInstr, &ctx);
    } else if (tok.match(Token("func", TokenType::KEYWORD_TOKEN)) ||
               tok.match(Token("static", TokenType::KEYWORD_TOKEN))) {
        FuncDeclContext ctx;
        ctx.visibility = currentVisibilityLevel;
        ctx.isOverride = currentOverrideLevel;
        if (tok.match(Token("static", TokenType::KEYWORD_TOKEN))) {
            ctx.isStatic = true;
            advance();
        }

        return parseInstruction(funcDeclInstr, &ctx);
    } else if (tok.match(Token("class", TokenType::KEYWORD_TOKEN))) {
        ClassDeclContext ctx;
        ctx.visibility = currentVisibilityLevel;
        return parseInstruction(classDeclInstr, &ctx);
    } else if (tok.match(Token("module", TokenType::KEYWORD_TOKEN))) {
        ModuleDeclContext ctx;
        return parseInstruction(moduleDeclInstr, &ctx);
    } else if (tok.match(Token("const", TokenType::KEYWORD_TOKEN))) {
        VarDeclContext ctx;
        return parseInstruction(constDeclInstr, &ctx);
    } else if (tok.match(Token("return", TokenType::KEYWORD_TOKEN))) {
        ReturnDeclContext ctx;
        return parseInstruction(returnDeclInstr, &ctx);
    } else if (tok.match(Token("@", TokenType::DELIMITER_TOKEN))) {
        AttributesContext ctx;
        return parseInstruction(attributesInstr, &ctx);
    } else if (tok.match(Token("import", TokenType::KEYWORD_TOKEN))) {
        ImportContext ctx;
        return parseInstruction(includeInstr, &ctx);
    }

    if (tok.match("private", TokenType::KEYWORD_TOKEN)) {
        currentVisibilityLevel = VisibilityKind::PRIVATE;
        advance();
        return parseExpr();
    } else if (tok.match("internal", TokenType::KEYWORD_TOKEN)) {
        currentVisibilityLevel = VisibilityKind::INTERNAL;
        advance();
        return parseExpr();
    } else if (tok.match("public", TokenType::KEYWORD_TOKEN)) {
        currentVisibilityLevel = VisibilityKind::PUBLIC;
        advance();
        return parseExpr();
    }

    if (tok.match("override", TokenType::KEYWORD_TOKEN)) {
        currentOverrideLevel = true;
        advance();
        return parseExpr();
    }

    auto node = parseTerm();

    while (true) {
        Token tok = getCurrent();
        if (tok.match(Token("+", TokenType::ARITHMETIC_TOKEN)) || tok.match(Token("-", TokenType::ARITHMETIC_TOKEN))) {
            std::string op = tok.value;
            advance();
            auto right = parseTerm();
            node = std::make_unique<BinaryNode>(PositionSpan(tok.position.start, getCurrent().position.end), op,
                                                std::move(node), std::move(right));
        } else {
            break;
        }
    }

    while (true) {
        Token tok = getCurrent();
        if (tok.match(Token("==", TokenType::COMPARISON_TOKEN)) ||
            tok.match(Token("!=", TokenType::COMPARISON_TOKEN)) || tok.match(Token("<", TokenType::COMPARISON_TOKEN)) ||
            tok.match(Token(">", TokenType::COMPARISON_TOKEN)) || tok.match(Token("<=", TokenType::COMPARISON_TOKEN)) ||
            tok.match(Token(">=", TokenType::COMPARISON_TOKEN))) {
            std::string op = tok.value;
            advance();
            auto right = parseTerm();
            node = std::make_unique<BinaryNode>(PositionSpan(tok.position.start, getCurrent().position.end), op,
                                                std::move(node), std::move(right));
        } else {
            break;
        }
    }

    if (getCurrent().match(Token("?", TokenType::DELIMITER_TOKEN))) {
        advance();
        if (getCurrent().match(Token(":", TokenType::DELIMITER_TOKEN))) {
            advance();
            auto right = parseExpr();
            return std::make_unique<NullCoalescingNode>(PositionSpan(node->position.start, right->position.end),
                                                        std::move(node), std::move(right));
        }
    }

    if (getCurrent().match(Token("=", TokenType::ASSIGNMENT_TOKEN))) {
        advance();
        auto right = parseExpr();
        return std::make_unique<VariableAssigment>(PositionSpan(node->position.start, right->position.end),
                                                   std::move(node), std::move(right));
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto node = parseFactor();

    while (true) {
        Token tok = getCurrent();

        if (tok.match(Token("*", TokenType::ARITHMETIC_TOKEN)) || tok.match(Token("/", TokenType::ARITHMETIC_TOKEN))) {
            std::string op = tok.value;
            advance();

            auto right = parseFactor();
            if (!node || !right) return nullptr;

            node = std::make_unique<BinaryNode>(PositionSpan(tok.position.start, getCurrent().position.end), op,
                                                std::move(node), std::move(right));
        } else {
            break;
        }
    }

    return node;
}

double strToDouble(std::string str) {
    if (str == "Infinity") return INFINITY;
    else if (str == "NaN") return NAN;
    else return std::stod(str);
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    Token tok = getCurrent();

    if (tok.type == TokenType::NUMBER_TOKEN) {
        advance();
        return std::make_unique<NumberNode>(PositionSpan(tok.position.start, tok.position.end), strToDouble(tok.value));
    }

    if (tok.type == TokenType::IDENTYFIER_TOKEN) {
        advance();
        return std::make_unique<IdentyfierNode>(tok.position, tok.value);
    }

    if (tok.match(Token("(", TokenType::DELIMITER_TOKEN))) {
        advance();
        auto node = parseExpr();
        if (!getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
            pushError(Error(tok.position, "Expected ')'", mainSource.filename().string()));
            return nullptr;
        }

        return node;
    }

    if (tok.match(Token("this", TokenType::KEYWORD_TOKEN))) {
        advance();
        return std::make_unique<ThisNode>(tok.position);
    }

    if (tok.match(Token("null", TokenType::KEYWORD_TOKEN))) {
        advance();
        return std::make_unique<NullNode>(tok.position);
    }

    if (tok.type == TokenType::STRING_TOKEN) {
        advance();
        return std::make_unique<StringNode>(PositionSpan(tok.position.start, tok.position.end), tok.value);
    }

    pushError(
        Error(tok.position, "Expected primary expression, got '" + tok.value + "'", mainSource.filename().string()));
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    Token tok = getCurrent();

    if (tok.match(Token("!", TokenType::DELIMITER_TOKEN)) || tok.match(Token("~", TokenType::DELIMITER_TOKEN))) {
        std::string op = tok.value;
        advance();
        auto value = parseFactor();
        if (!value) return nullptr;
        return std::make_unique<UnaryNode>(PositionSpan(tok.position.start, value->position.end), op, std::move(value));
    }

    auto node = parsePrimary();
    if (!node) return nullptr;

    if (!allowCallAndMember) return node;

    while (true) {
        tok = getCurrent();

        if (tok.match(Token("(", TokenType::DELIMITER_TOKEN))) {
            advance();

            std::vector<std::unique_ptr<ASTNode>> args;
            if (!getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
                do {
                    auto arg = parseExpr();
                    if (!arg) return nullptr;
                    args.push_back(std::move(arg));

                    if (getCurrent().match(Token(",", TokenType::DELIMITER_TOKEN))) {
                        advance();
                    } else {
                        break;
                    }
                } while (true);
            }

            if (!getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
                pushError(Error(tok.position, "Expected ')'", mainSource.filename().string()));
                return nullptr;
            }
            advance();

            node = std::make_unique<CallNode>(PositionSpan(node->position.start, getCurrent().position.end),
                                              std::move(node), std::move(args));
            continue;
        }

        if (tok.match(Token(".", TokenType::DELIMITER_TOKEN))) {
            advance();

            Token memberTok = getCurrent();
            if (memberTok.type != TokenType::IDENTYFIER_TOKEN) {
                pushError(Error(memberTok.position, "Expected identifier after '.'", mainSource.filename().string()));
                return nullptr;
            }
            advance();

            auto memberNode = std::make_unique<IdentyfierNode>(memberTok.position, memberTok.value);

            node = std::make_unique<MemberAccessNode>(PositionSpan(node->position.start, memberNode->position.end),
                                                      std::move(node), std::move(memberNode));
            continue;
        }

        if (tok.match(Token("?.", TokenType::DELIMITER_TOKEN))) {
            advance();

            Token memberTok = getCurrent();
            if (memberTok.type != TokenType::IDENTYFIER_TOKEN) {
                pushError(Error(memberTok.position, "Expected identifier after '?.'", mainSource.filename().string()));
                return nullptr;
            }
            advance();

            auto memberNode = std::make_unique<IdentyfierNode>(memberTok.position, memberTok.value);

            node = std::make_unique<SafeNavigationNode>(PositionSpan(node->position.start, memberNode->position.end),
                                                        std::move(node), std::move(memberNode));
            continue;
        }

        break;
    }

    if (getCurrent().match(Token("??", TokenType::DELIMITER_TOKEN))) {
        advance();
        node = std::make_unique<NullCheckNode>(PositionSpan(node->position.start, getCurrent().position.end),
                                               std::move(node));
    }

    return node;
}
