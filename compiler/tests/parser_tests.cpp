#include "parser/parser.hpp"
#include "lexer.hpp"
#include "parser_tests.hpp"

PARSER_TEST(BinaryExpression) {
    std::vector<Token> tokens = {
        Token("1", TokenType::NUMBER_TOKEN),
        Token("+", TokenType::ARITHMETIC_TOKEN),
        Token("2", TokenType::NUMBER_TOKEN),
        Token("*", TokenType::ARITHMETIC_TOKEN),
        Token("3", TokenType::NUMBER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };

    Parser parser(tokens);
    auto nodes = parser.parse();

    auto errors = parser.getErrors();
    ASSERT_TRUE(errors.empty());

    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto expr = dynamic_cast<BinaryNode*>(stmt->value.get());
    ASSERT_NE(expr, nullptr);
    ASSERT_EQ(expr->op, "+");

    auto rightMul = dynamic_cast<BinaryNode*>(expr->right.get());
    ASSERT_NE(rightMul, nullptr);

    ASSERT_EQ(rightMul->op, "*");

    auto leftNum = dynamic_cast<NumberNode*>(expr->left.get());
    auto leftMulNum = dynamic_cast<NumberNode*>(rightMul->left.get());
    auto rightMulNum = dynamic_cast<NumberNode*>(rightMul->right.get());

    ASSERT_NE(leftNum, nullptr);
    ASSERT_NE(leftMulNum, nullptr);
    ASSERT_NE(rightMulNum, nullptr);

    ASSERT_EQ(leftNum->value, 1.0);
    ASSERT_EQ(leftMulNum->value, 2.0);
    ASSERT_EQ(rightMulNum->value, 3.0);
}

PARSER_TEST(VariableDeclaration) {
    {
        std::vector<Token> tokens = {
            Token("let", TokenType::KEYWORD_TOKEN),
            Token("a", TokenType::IDENTYFIER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("Int", TokenType::IDENTYFIER_TOKEN),
            Token("=", TokenType::ASSIGNMENT_TOKEN),
            Token("2", TokenType::NUMBER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto expr = dynamic_cast<VariableDeclarationNode*>(stmt->value.get());
        ASSERT_NE(expr, nullptr);
        ASSERT_EQ(expr->isConst, false);

        auto name = dynamic_cast<IdentyfierNode*>(expr->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "a");

        auto value = dynamic_cast<NumberNode*>(expr->value.get());
        ASSERT_NE(value, nullptr);
        ASSERT_EQ(value->value, 2.0);

        auto type = dynamic_cast<IdentyfierNode*>(expr->type.get());
        ASSERT_NE(type, nullptr);
        ASSERT_EQ(type->value, "Int");
    }
    {
        std::vector<Token> tokens = {
            Token("let", TokenType::KEYWORD_TOKEN),
            Token("b", TokenType::IDENTYFIER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("Int", TokenType::IDENTYFIER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto expr = dynamic_cast<VariableDeclarationNode*>(stmt->value.get());
        ASSERT_NE(expr, nullptr);
        ASSERT_EQ(expr->isConst, false);

        auto name = dynamic_cast<IdentyfierNode*>(expr->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "b");

        ASSERT_EQ(expr->value.get(), nullptr);

        auto type = dynamic_cast<IdentyfierNode*>(expr->type.get());
        ASSERT_NE(type, nullptr);
        ASSERT_EQ(type->value, "Int");
    }
}

PARSER_TEST(ConstantDeclaration) {
    {
        std::vector<Token> tokens = {
            Token("const", TokenType::KEYWORD_TOKEN),
            Token("a", TokenType::IDENTYFIER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("Int", TokenType::IDENTYFIER_TOKEN),
            Token("=", TokenType::ASSIGNMENT_TOKEN),
            Token("2", TokenType::NUMBER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto expr = dynamic_cast<VariableDeclarationNode*>(stmt->value.get());
        ASSERT_NE(expr, nullptr);
        ASSERT_EQ(expr->isConst, true);

        auto name = dynamic_cast<IdentyfierNode*>(expr->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "a");

        auto value = dynamic_cast<NumberNode*>(expr->value.get());
        ASSERT_NE(value, nullptr);
        ASSERT_EQ(value->value, 2.0);

        auto type = dynamic_cast<IdentyfierNode*>(expr->type.get());
        ASSERT_NE(type, nullptr);
        ASSERT_EQ(type->value, "Int");
    }
    {
        std::vector<Token> tokens = {
            Token("const", TokenType::KEYWORD_TOKEN),
            Token("b", TokenType::IDENTYFIER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("Int", TokenType::IDENTYFIER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto expr = dynamic_cast<VariableDeclarationNode*>(stmt->value.get());
        ASSERT_NE(expr, nullptr);
        ASSERT_EQ(expr->isConst, true);

        auto name = dynamic_cast<IdentyfierNode*>(expr->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "b");

        ASSERT_EQ(expr->value.get(), nullptr);

        auto type = dynamic_cast<IdentyfierNode*>(expr->type.get());
        ASSERT_NE(type, nullptr);
        ASSERT_EQ(type->value, "Int");
    }
}

PARSER_TEST(VariableAssigment) {
    std::vector<Token> tokens = {
        Token("a", TokenType::IDENTYFIER_TOKEN),
        Token("=", TokenType::ASSIGNMENT_TOKEN),
        Token("15", TokenType::NUMBER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };

    Parser parser(tokens);
    auto nodes = parser.parse();

    auto errors = parser.getErrors();
    ASSERT_TRUE(errors.empty());

    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto expr = dynamic_cast<VariableAssigment*>(stmt->value.get());
    ASSERT_NE(expr, nullptr);

    auto name = dynamic_cast<IdentyfierNode*>(expr->name.get());
    ASSERT_NE(name, nullptr);
    ASSERT_EQ(name->value, "a");

    auto value = dynamic_cast<NumberNode*>(expr->value.get());
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(value->value, 15.0);
}

PARSER_TEST(FunctionDeclaration) {
    {
        std::vector<Token> tokens = {
            Token("func", TokenType::KEYWORD_TOKEN),
            Token("a", TokenType::IDENTYFIER_TOKEN),
            Token("(", TokenType::DELIMITER_TOKEN),
            Token("b", TokenType::IDENTYFIER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("Int", TokenType::IDENTYFIER_TOKEN),
            Token(")", TokenType::DELIMITER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("Int", TokenType::IDENTYFIER_TOKEN),
            Token("{", TokenType::DELIMITER_TOKEN),
            Token("}", TokenType::DELIMITER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto func = dynamic_cast<FunctionDeclaration*>(stmt->value.get());
        ASSERT_NE(func, nullptr);

        auto name = dynamic_cast<IdentyfierNode*>(func->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "a");

        auto retType = dynamic_cast<IdentyfierNode*>(func->type.get());
        ASSERT_NE(retType, nullptr);
        ASSERT_EQ(retType->value, "Int");

        ASSERT_EQ(func->parameters.size(), 1);

        auto arg0 = dynamic_cast<ArgDeclaration*>(func->parameters[0].get());
        ASSERT_NE(arg0, nullptr);

        auto argName = dynamic_cast<IdentyfierNode*>(arg0->name.get());
        ASSERT_NE(argName, nullptr);
        ASSERT_EQ(argName->value, "b");

        auto argType = dynamic_cast<IdentyfierNode*>(arg0->type.get());
        ASSERT_NE(argType, nullptr);
        ASSERT_EQ(argType->value, "Int");

        ASSERT_EQ(func->body.size(), 0);
        ASSERT_EQ(func->isForward, false);
        ASSERT_EQ(func->isStatic, false);
    }
    {
        std::vector<Token> tokens = {
            Token("func", TokenType::KEYWORD_TOKEN),
            Token("a", TokenType::IDENTYFIER_TOKEN),
            Token("(", TokenType::DELIMITER_TOKEN),
            Token("b", TokenType::IDENTYFIER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("Int", TokenType::IDENTYFIER_TOKEN),
            Token(")", TokenType::DELIMITER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("Int", TokenType::IDENTYFIER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto func = dynamic_cast<FunctionDeclaration*>(stmt->value.get());
        ASSERT_NE(func, nullptr);

        auto name = dynamic_cast<IdentyfierNode*>(func->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "a");

        auto retType = dynamic_cast<IdentyfierNode*>(func->type.get());
        ASSERT_NE(retType, nullptr);
        ASSERT_EQ(retType->value, "Int");

        ASSERT_EQ(func->parameters.size(), 1);

        auto arg0 = dynamic_cast<ArgDeclaration*>(func->parameters[0].get());
        ASSERT_NE(arg0, nullptr);

        auto argName = dynamic_cast<IdentyfierNode*>(arg0->name.get());
        ASSERT_NE(argName, nullptr);
        ASSERT_EQ(argName->value, "b");

        auto argType = dynamic_cast<IdentyfierNode*>(arg0->type.get());
        ASSERT_NE(argType, nullptr);
        ASSERT_EQ(argType->value, "Int");

        ASSERT_EQ(func->body.size(), 0);
        ASSERT_EQ(func->isForward, true);
        ASSERT_EQ(func->isStatic, false);
    }
    {
        std::vector<Token> tokens = {
            Token("static", TokenType::KEYWORD_TOKEN),
            Token("func", TokenType::KEYWORD_TOKEN),
            Token("a", TokenType::IDENTYFIER_TOKEN),
            Token("(", TokenType::DELIMITER_TOKEN),
            Token("b", TokenType::IDENTYFIER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("Int", TokenType::IDENTYFIER_TOKEN),
            Token(")", TokenType::DELIMITER_TOKEN),
            Token(":", TokenType::DELIMITER_TOKEN),
            Token("Int", TokenType::IDENTYFIER_TOKEN),
            Token("{", TokenType::DELIMITER_TOKEN),
            Token("}", TokenType::DELIMITER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto func = dynamic_cast<FunctionDeclaration*>(stmt->value.get());
        ASSERT_NE(func, nullptr);

        auto name = dynamic_cast<IdentyfierNode*>(func->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "a");

        auto retType = dynamic_cast<IdentyfierNode*>(func->type.get());
        ASSERT_NE(retType, nullptr);
        ASSERT_EQ(retType->value, "Int");

        ASSERT_EQ(func->parameters.size(), 1);

        auto arg0 = dynamic_cast<ArgDeclaration*>(func->parameters[0].get());
        ASSERT_NE(arg0, nullptr);

        auto argName = dynamic_cast<IdentyfierNode*>(arg0->name.get());
        ASSERT_NE(argName, nullptr);
        ASSERT_EQ(argName->value, "b");

        auto argType = dynamic_cast<IdentyfierNode*>(arg0->type.get());
        ASSERT_NE(argType, nullptr);
        ASSERT_EQ(argType->value, "Int");

        ASSERT_EQ(func->body.size(), 0);
        ASSERT_EQ(func->isForward, false);
        ASSERT_EQ(func->isStatic, true);
    }
}

PARSER_TEST(ClassDeclaration) {
    {
        std::vector<Token> tokens = {
            Token("class", TokenType::KEYWORD_TOKEN),
            Token("Program", TokenType::IDENTYFIER_TOKEN),
            Token("{", TokenType::DELIMITER_TOKEN),
            Token("}", TokenType::DELIMITER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto classNode = dynamic_cast<ClassDeclNode*>(stmt->value.get());
        ASSERT_NE(classNode, nullptr);

        auto name = dynamic_cast<IdentyfierNode*>(classNode->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "Program");

        ASSERT_EQ(classNode->members.size(), 0);
        ASSERT_EQ(classNode->isForward, false);
    }
    {
        std::vector<Token> tokens = {
            Token("class", TokenType::KEYWORD_TOKEN),
            Token("Program", TokenType::IDENTYFIER_TOKEN),
            Token(";", TokenType::DELIMITER_TOKEN),
            Token("", TokenType::EOF_TOKEN)
        };

        Parser parser(tokens);
        auto nodes = parser.parse();

        auto errors = parser.getErrors();
        ASSERT_TRUE(errors.empty());
        ASSERT_EQ(nodes.size(), 1);

        auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
        ASSERT_NE(stmt, nullptr);

        auto classNode = dynamic_cast<ClassDeclNode*>(stmt->value.get());
        ASSERT_NE(classNode, nullptr);

        auto name = dynamic_cast<IdentyfierNode*>(classNode->name.get());
        ASSERT_NE(name, nullptr);
        ASSERT_EQ(name->value, "Program");

        ASSERT_EQ(classNode->members.size(), 0);
        ASSERT_EQ(classNode->isForward, true);
    }
}

PARSER_TEST(ModuleDeclaration) {
    std::vector<Token> tokens = {
        Token("module", TokenType::KEYWORD_TOKEN),
        Token("main", TokenType::IDENTYFIER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };
    Parser parser(tokens);
    auto nodes = parser.parse();

    auto errors = parser.getErrors();
    ASSERT_TRUE(errors.empty());

    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto moduleNode = dynamic_cast<ModuleDeclaration*>(stmt->value.get());
    ASSERT_NE(moduleNode, nullptr);

    auto name = dynamic_cast<IdentyfierNode*>(moduleNode->name.get());
    ASSERT_NE(name, nullptr);
    ASSERT_EQ(name->value, "main");
}

PARSER_TEST(FunctionCall) {
    std::vector<Token> tokens = {
        Token("abc", TokenType::IDENTYFIER_TOKEN),
        Token("(", TokenType::DELIMITER_TOKEN),
        Token(")", TokenType::DELIMITER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };
    Parser parser(tokens);
    auto nodes = parser.parse();

    auto errors = parser.getErrors();
    ASSERT_TRUE(errors.empty());

    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto callNode = dynamic_cast<CallNode*>(stmt->value.get());
    ASSERT_NE(callNode, nullptr);
    ASSERT_EQ(callNode->args.size(), 0);

    auto callee = dynamic_cast<IdentyfierNode*>(callNode->callee.get());
    ASSERT_EQ(callee->value, "abc");
}

PARSER_TEST(MemberAccess) {
    std::vector<Token> tokens = {
        Token("abc", TokenType::IDENTYFIER_TOKEN),
        Token(".", TokenType::DELIMITER_TOKEN),
        Token("cba", TokenType::IDENTYFIER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };
    Parser parser(tokens);
    auto nodes = parser.parse();

    auto errors = parser.getErrors();
    ASSERT_TRUE(errors.empty());

    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto memberNode = dynamic_cast<MemberAccessNode*>(stmt->value.get());
    ASSERT_NE(memberNode, nullptr);

    auto member = dynamic_cast<IdentyfierNode*>(memberNode->member.get());
    auto object = dynamic_cast<IdentyfierNode*>(memberNode->object.get());

    ASSERT_NE(member, nullptr);
    ASSERT_NE(object, nullptr);

    ASSERT_EQ(object->value, "abc");
    ASSERT_EQ(member->value, "cba");
}