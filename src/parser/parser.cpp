#include "parser/parser.hpp"
#include <exception>
#include <iostream>
#include "parser/context.hpp"

struct ASTNode;

Parser::Parser(std::vector<Token> toks): tokens(toks) {
    initVarDecl();
    initFuncDecl();
    initClassDecl();
    initModuleDecl();
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
        pushError(Error(getCurrent().position, "Expected '(' at beginning of function arguments"));
        return args;
    }
    advance();

    if (!(getCurrent().match(Token("arg", TokenType::KEYWORD_TOKEN)) || getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN)))) {
        pushError(Error(getCurrent().position, "Expected argument at function arguments"));
        return args;
    } else if (getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
        return args;
    }
    advance();

    while (!getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
        Token argNameToken = getCurrent();
        if (argNameToken.type != TokenType::IDENTYFIER_TOKEN) {
            pushError(Error(argNameToken.position, "Expected argument name"));
            return args;
        }
        advance();

        if (!getCurrent().match(Token(":", TokenType::DELIMITER_TOKEN))) {
            pushError(Error(getCurrent().position, "Expected ':' after argument name"));
            return args;
        }
        advance();

        Token typeToken = getCurrent();
        if (typeToken.type != TokenType::IDENTYFIER_TOKEN) {
            pushError(Error(typeToken.position, "Expected type for argument"));
            return args;
        }
        advance();

        args.push_back(std::make_unique<ArgDeclaration>(
            PositionSpan(argNameToken.position.start, typeToken.position.end),
            std::make_unique<IdentyfierNode>(argNameToken.position, argNameToken.value),
            std::make_unique<IdentyfierNode>(typeToken.position, typeToken.value)
        ));

        if (getCurrent().match(Token(",", TokenType::DELIMITER_TOKEN))) {
            advance();
        } else if (!getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
            pushError(Error(getCurrent().position, "Expected ',' or ')' in argument list"));
            return args;
        }
    }

    return args;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parseBlock() {
    std::vector<std::unique_ptr<ASTNode>> statements;

    if (!getCurrent().match(Token("{", TokenType::DELIMITER_TOKEN))) {
        pushError(Error(getCurrent().position, "Expected '{' at beginning of block"));
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
        pushError(Error(getCurrent().position, "Expected '}' at end of block"));
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
                pushError(Error(tok.position, "Unexpected token: " + tok.value));
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
    auto node = parseExpr();
    if (!node) {
        pushError(Error(getCurrent().position, "Invalid expression in statement"));
        return nullptr;
    }

    if (!getCurrent().match(Token(";", TokenType::DELIMITER_TOKEN))) {
        pushError(Error(PositionSpan(getCurrent().position.start, getCurrent().position.end), "Expected ';' after statement"));
        return nullptr;
    }
    advance();
    return std::make_unique<StatementNode>(PositionSpan(node->position.start, getCurrent().position.end), std::move(node));
}

std::unique_ptr<ASTNode> Parser::parseExpr() {
    Token tok = getCurrent();
 
    if (tok.match(Token("let", TokenType::KEYWORD_TOKEN))) {
        VarDeclContext ctx;
        return parseInstruction(varDeclInstr, &ctx);
    } else if (tok.match(Token("func", TokenType::KEYWORD_TOKEN)) || tok.match(Token("static", TokenType::KEYWORD_TOKEN))) {
        FuncDeclContext ctx;
        if (tok.match(Token("static", TokenType::KEYWORD_TOKEN))) {
            ctx.isStatic = true;
            advance();
        }
        
        return parseInstruction(funcDeclInstr, &ctx);
    } else if (tok.match(Token("class", TokenType::KEYWORD_TOKEN))) {
        ClassDeclContext ctx;
        return parseInstruction(classDeclInstr, &ctx);
    } else if (tok.match(Token("module", TokenType::KEYWORD_TOKEN))) {
        ModuleDeclContext ctx;
        return parseInstruction(moduleDeclInstr, &ctx);
    }
    
    auto node = parseTerm();

    while (true) {
        Token tok = getCurrent();
        if (tok.match(Token("+", TokenType::ARITHMETIC_TOKEN)) || tok.match(Token("-", TokenType::ARITHMETIC_TOKEN))) {
            std::string op = tok.value;
            advance();
            auto right = parseTerm();
            node = std::make_unique<BinaryNode>(PositionSpan(tok.position.start, getCurrent().position.end), op, std::move(node), std::move(right));
        } else {
            break;
        }
    }

    if (getCurrent().match(Token("=", TokenType::ASSIGNMENT_TOKEN))) {
        advance();
        auto right = parseExpr();
        return std::make_unique<VariableAssigment>(PositionSpan(node->position.start, right->position.end), std::move(node), std::move(right));
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto node = parseFactor();

    while (true) {
        Token tok = getCurrent();

        if (tok.match(Token("*", TokenType::ARITHMETIC_TOKEN)) ||
            tok.match(Token("/", TokenType::ARITHMETIC_TOKEN))) {
            std::string op = tok.value;
            advance();

            auto right = parseFactor();
            if (!node || !right) return nullptr;

            node = std::make_unique<BinaryNode>(PositionSpan(tok.position.start, getCurrent().position.end), op, std::move(node), std::move(right));
        } else {
            break;
        }
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    Token tok = getCurrent();

    if (tok.type == TokenType::NUMBER_TOKEN) {
        advance();
        return std::make_unique<NumberNode>(
            PositionSpan(tok.position.start, tok.position.end),
            std::stod(tok.value)
        );
    }

    if (tok.type == TokenType::IDENTYFIER_TOKEN) {
        advance();
        return std::make_unique<IdentyfierNode>(tok.position, tok.value);
    }

    if (tok.match(Token("(", TokenType::DELIMITER_TOKEN))) {
        advance();
        auto node = parseExpr();
        if (!getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
            pushError(Error(tok.position, "Expected ')'"));
            return nullptr;
        }

        return node;
    }

    std::cout << "Error at position " << tok.position.start.line << ":" << tok.position.start.column << " - Unexpected token: " << tok.value << std::endl;
    pushError(Error(tok.position, "Expected primary expression"));
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    auto node = parsePrimary();
    if (!node) return nullptr;

    if (!allowCallAndMember)
        return node;

    while (true) {
        Token tok = getCurrent();

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
                pushError(Error(tok.position, "Expected ')'"));
                return nullptr;
            }
            advance();

            node = std::make_unique<CallNode>(
                PositionSpan(node->position.start, getCurrent().position.end),
                std::move(node),
                std::move(args)
            );
            continue;
        }

        if (tok.match(Token(".", TokenType::DELIMITER_TOKEN))) {
            advance();

            Token memberTok = getCurrent();
            if (memberTok.type != TokenType::IDENTYFIER_TOKEN) {
                pushError(Error(memberTok.position, "Expected identifier after '.'"));
                return nullptr;
            }
            advance();

            auto memberNode = std::make_unique<IdentyfierNode>(memberTok.position, memberTok.value);

            node = std::make_unique<MemberAccessNode>(
                PositionSpan(node->position.start, memberNode->position.end),
                std::move(node),
                std::move(memberNode)
            );
            continue;
        }

        break;
    }

    return node;
}
