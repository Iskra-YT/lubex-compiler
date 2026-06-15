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

    PARSER_TEST(ReturnStatement) {
    std::vector<Token> tokens = {
        Token("return", TokenType::KEYWORD_TOKEN),
        Token("42", TokenType::NUMBER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };

    Parser parser(tokens);
    auto nodes = parser.parse();

    ASSERT_TRUE(parser.getErrors().empty());
    ASSERT_EQ(nodes.size(), 1);

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    ASSERT_NE(stmt, nullptr);

    auto ret = dynamic_cast<ReturnNode*>(stmt->value.get());
    ASSERT_NE(ret, nullptr);

    auto val = dynamic_cast<NumberNode*>(ret->value.get());
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(val->value, 42.0);
    }

PARSER_TEST(NullableType) {
    std::vector<Token> tokens = {
        Token("let", TokenType::KEYWORD_TOKEN),
        Token("x", TokenType::IDENTYFIER_TOKEN),
        Token(":", TokenType::DELIMITER_TOKEN),
        Token("Number", TokenType::IDENTYFIER_TOKEN),
        Token("?", TokenType::DELIMITER_TOKEN),
        Token("=", TokenType::ASSIGNMENT_TOKEN),
        Token("null", TokenType::KEYWORD_TOKEN),
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

    auto varDecl = dynamic_cast<VariableDeclarationNode*>(stmt->value.get());
    ASSERT_NE(varDecl, nullptr);
    ASSERT_EQ(varDecl->isConst, false);

    auto nullableType = dynamic_cast<NullableTypeNode*>(varDecl->type.get());
    ASSERT_NE(nullableType, nullptr);

    auto baseType = dynamic_cast<IdentyfierNode*>(nullableType->baseType.get());
    ASSERT_NE(baseType, nullptr);
    ASSERT_EQ(baseType->value, "Number");

    auto value = dynamic_cast<NullNode*>(varDecl->value.get());
    ASSERT_NE(value, nullptr);
}

PARSER_TEST(NullCheck) {
    std::vector<Token> tokens = {
        Token("x", TokenType::IDENTYFIER_TOKEN),
        Token("??", TokenType::DELIMITER_TOKEN),
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

    auto nullCheck = dynamic_cast<NullCheckNode*>(stmt->value.get());
    ASSERT_NE(nullCheck, nullptr);

    auto val = dynamic_cast<IdentyfierNode*>(nullCheck->value.get());
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(val->value, "x");
}

PARSER_TEST(NullCoalescing) {
    std::vector<Token> tokens = {
        Token("x", TokenType::IDENTYFIER_TOKEN),
        Token("?", TokenType::DELIMITER_TOKEN),
        Token(":", TokenType::DELIMITER_TOKEN),
        Token("y", TokenType::IDENTYFIER_TOKEN),
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

    auto coalesce = dynamic_cast<NullCoalescingNode*>(stmt->value.get());
    ASSERT_NE(coalesce, nullptr);

    auto left = dynamic_cast<IdentyfierNode*>(coalesce->left.get());
    auto right = dynamic_cast<IdentyfierNode*>(coalesce->right.get());
    ASSERT_NE(left, nullptr);
    ASSERT_NE(right, nullptr);
    ASSERT_EQ(left->value, "x");
    ASSERT_EQ(right->value, "y");
}

PARSER_TEST(SafeNavigation) {
    std::vector<Token> tokens = {
        Token("x", TokenType::IDENTYFIER_TOKEN),
        Token("?.", TokenType::DELIMITER_TOKEN),
        Token("y", TokenType::IDENTYFIER_TOKEN),
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

    auto safeNav = dynamic_cast<SafeNavigationNode*>(stmt->value.get());
    ASSERT_NE(safeNav, nullptr);

    auto object = dynamic_cast<IdentyfierNode*>(safeNav->object.get());
    auto member = dynamic_cast<IdentyfierNode*>(safeNav->member.get());
    ASSERT_NE(object, nullptr);
    ASSERT_NE(member, nullptr);
    ASSERT_EQ(object->value, "x");
    ASSERT_EQ(member->value, "y");
}

PARSER_TEST(ImportStatement) {
    std::vector<Token> tokens = {
        Token("import", TokenType::KEYWORD_TOKEN),
        Token("std", TokenType::IDENTYFIER_TOKEN),
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

    auto importNode = dynamic_cast<ImportNode*>(stmt->value.get());
    ASSERT_NE(importNode, nullptr);

    auto moduleName = dynamic_cast<IdentyfierNode*>(importNode->value.get());
    ASSERT_NE(moduleName, nullptr);
    ASSERT_EQ(moduleName->value, "std");
}

PARSER_TEST(Attributes) {
    std::vector<Token> tokens = {
        Token("@", TokenType::DELIMITER_TOKEN),
        Token("mangle", TokenType::IDENTYFIER_TOKEN),
        Token("(", TokenType::DELIMITER_TOKEN),
        Token("testName", TokenType::IDENTYFIER_TOKEN),
        Token(")", TokenType::DELIMITER_TOKEN),
        Token("class", TokenType::KEYWORD_TOKEN),
        Token("Foo", TokenType::IDENTYFIER_TOKEN),
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

    auto attrNode = dynamic_cast<AttributesNode*>(stmt->value.get());
    ASSERT_NE(attrNode, nullptr);

    auto attrName = dynamic_cast<IdentyfierNode*>(attrNode->name.get());
    ASSERT_NE(attrName, nullptr);
    ASSERT_EQ(attrName->value, "mangle");

    ASSERT_EQ(attrNode->params.size(), 1);

    auto classDecl = dynamic_cast<ClassDeclNode*>(attrNode->value.get());
    ASSERT_NE(classDecl, nullptr);

    auto className = dynamic_cast<IdentyfierNode*>(classDecl->name.get());
    ASSERT_NE(className, nullptr);
    ASSERT_EQ(className->value, "Foo");
}

PARSER_TEST(ClassWithMembers) {
    std::vector<Token> tokens = {
        Token("class", TokenType::KEYWORD_TOKEN),
        Token("Foo", TokenType::IDENTYFIER_TOKEN),
        Token("{", TokenType::DELIMITER_TOKEN),
        Token("let", TokenType::KEYWORD_TOKEN),
        Token("x", TokenType::IDENTYFIER_TOKEN),
        Token(":", TokenType::DELIMITER_TOKEN),
        Token("Number", TokenType::IDENTYFIER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
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
    ASSERT_EQ(name->value, "Foo");

    ASSERT_EQ(classNode->members.size(), 1);
    ASSERT_EQ(classNode->isForward, false);
}

PARSER_TEST(Inheritance) {
    std::vector<Token> tokens = {
        Token("class", TokenType::KEYWORD_TOKEN),
        Token("B", TokenType::IDENTYFIER_TOKEN),
        Token("extends", TokenType::KEYWORD_TOKEN),
        Token("A", TokenType::IDENTYFIER_TOKEN),
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
    ASSERT_EQ(name->value, "B");

    auto parent = dynamic_cast<IdentyfierNode*>(classNode->parent.get());
    ASSERT_NE(parent, nullptr);
    ASSERT_EQ(parent->value, "A");

    ASSERT_EQ(classNode->isForward, false);
}

PARSER_TEST(ThisExpression) {
    std::vector<Token> tokens = {
        Token("this", TokenType::KEYWORD_TOKEN),
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

    auto thisNode = dynamic_cast<ThisNode*>(stmt->value.get());
    ASSERT_NE(thisNode, nullptr);
}

PARSER_TEST(StringLiteral) {
    std::vector<Token> tokens = {
        Token("hello", TokenType::STRING_TOKEN),
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

    auto strNode = dynamic_cast<StringNode*>(stmt->value.get());
    ASSERT_NE(strNode, nullptr);
    ASSERT_EQ(strNode->value, "hello");
}

PARSER_TEST(ParserErrorHandling) {
    std::vector<Token> tokens = {
        Token("class", TokenType::KEYWORD_TOKEN),
        Token("Foo", TokenType::IDENTYFIER_TOKEN),
        Token("{", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };

    Parser parser(tokens);
    auto nodes = parser.parse();

    auto errors = parser.getErrors();
    ASSERT_FALSE(errors.empty());
}

    PARSER_TEST(ComplexBinaryExpressionPrecedence) {
    // 1 + 2 * 3 + 4
    std::vector<Token> tokens = {
        Token("1", TokenType::NUMBER_TOKEN),
        Token("+", TokenType::ARITHMETIC_TOKEN),
        Token("2", TokenType::NUMBER_TOKEN),
        Token("*", TokenType::ARITHMETIC_TOKEN),
        Token("3", TokenType::NUMBER_TOKEN),
        Token("+", TokenType::ARITHMETIC_TOKEN),
        Token("4", TokenType::NUMBER_TOKEN),
        Token(";", TokenType::DELIMITER_TOKEN),
        Token("", TokenType::EOF_TOKEN)
    };

    Parser parser(tokens);
    auto nodes = parser.parse();

    ASSERT_TRUE(parser.getErrors().empty());

    auto stmt = dynamic_cast<StatementNode*>(nodes[0].get());
    auto root = dynamic_cast<BinaryNode*>(stmt->value.get());
    ASSERT_NE(root, nullptr);
    ASSERT_EQ(root->op, "+");

    auto left = dynamic_cast<BinaryNode*>(root->left.get());
    ASSERT_NE(left, nullptr);
    ASSERT_EQ(left->op, "+");

    auto right4 = dynamic_cast<NumberNode*>(root->right.get());
    ASSERT_NE(right4, nullptr);
    ASSERT_EQ(right4->value, 4.0);
    }