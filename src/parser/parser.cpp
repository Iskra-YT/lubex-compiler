#include "parser/parser.hpp"
#include <exception>
#include <iostream>
#include "parser/context.hpp"

Parser::Parser(std::vector<Token> toks): tokens(toks) {
    initVarDecl();
    initFuncDecl();
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

    if (!getCurrent().match(Token("arg", TokenType::KEYWORD_TOKEN))) {
        pushError(Error(getCurrent().position, "Expected argument at function arguments"));
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
    bool lastMatched = true;

    for (auto& step : instrSet.steps) {
        Token tok = getCurrent();

        bool matchesType = (step.expectedType == TokenType::ANY || tok.type == step.expectedType);
        bool matchesValue = (step.expectedValue.empty() || tok.value == step.expectedValue);

        bool matched = matchesType && matchesValue;

        if (!matched) {
            if (step.optional) {
                lastMatched = false;
                continue;
            } else {
                pushError(Error(tok.position, "Unexpected token: " + tok.value));
                return nullptr;
            }
        }
        if (step.expectedType == TokenType::ANY && step.optional && !lastMatched) {
            continue;
        }

        lastMatched = true;

        step.action(tok, context);

        if (!step.consumesToken) {
            advance();
        }
    }

    return instrSet.finalize(PositionSpan(start, getCurrent().position.end), context);
}


std::unique_ptr<ASTNode> Parser::parseStatement() {
    auto node = parseExpr();
    if (!node) {
        pushError(Error(getCurrent().position, "Invalid expression in statement"));
        return nullptr;
    }

    if (!getCurrent().match(Token(";", TokenType::DELIMITER_TOKEN))) {
        pushError(Error(PositionSpan(node->position.start, getCurrent().position.end), "Expected ';' after statement"));
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
    } else if (tok.match(Token("func", TokenType::KEYWORD_TOKEN))) {
        FuncDeclContext ctx;
        return parseInstruction(funcDeclInstr, &ctx);
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

std::unique_ptr<ASTNode> Parser::parseFactor() {
    Token tok = getCurrent();

    if (tok.type == TokenType::NUMBER_TOKEN) {
        advance();
        return std::make_unique<NumberNode>(PositionSpan(tok.position.start, getCurrent().position.end), std::stod(tok.value));
    }

    if (tok.match(Token("(", TokenType::DELIMITER_TOKEN))) {
        advance();
        auto node = parseExpr();
        if (!getCurrent().match(Token(")", TokenType::DELIMITER_TOKEN))) {
            pushError(Error(tok.position, "Expected ')'"));
            return nullptr;
        }
        advance();
        return node;
    }

    if (tok.match(Token("-", TokenType::ARITHMETIC_TOKEN))) {
        advance();
        auto right = parseFactor();
        auto zero = std::make_unique<NumberNode>(PositionSpan(0, 0), 0.0);
        return std::make_unique<BinaryNode>(PositionSpan(tok.position.start, getCurrent().position.end), "-", std::move(zero), std::move(right));
    }

    if (tok.match(Token("+", TokenType::ARITHMETIC_TOKEN))) {
        advance();
        return parseFactor();
    }

    if (tok.type == TokenType::IDENTYFIER_TOKEN) {
        advance();
        return std::make_unique<IdentyfierNode>(tok.position, tok.value);
    }

    pushError(Error(tok.position, "Unexpected token in factor: " + tok.value));
    advance();
    return nullptr;
}