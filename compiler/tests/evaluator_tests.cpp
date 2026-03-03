#include "evaluator_tests.hpp"
#include "evaluator.hpp"

#include "evaluator_tests.hpp"
#include "evaluator.hpp"

EVALUATOR_TEST(VariableDeclaration) {
    Context ctx(nullptr);
    ctx.phase = PassPhase::DECLARATION;
    auto numberIdent = IdentyfierNode(PositionSpan(0, 0), "Number");
    ctx.declare(std::make_unique<Symbol>(SymbolKind::CLASS, &numberIdent, nullptr, static_cast<ASTNode*>(&numberIdent)));

    auto name = std::make_unique<IdentyfierNode>(PositionSpan(0,0), "x");
    auto value = std::make_unique<NumberNode>(PositionSpan(0,0), 42);
    auto type = std::make_unique<IdentyfierNode>(PositionSpan(0,0), "Number");

    auto varDecl = std::make_unique<VariableDeclarationNode>(
        PositionSpan(0,0),
        std::move(name),
        std::move(value),
        std::move(type),
        false
    );

    Symbol* sym = varDecl->evaluateSymbol(ctx);

    ASSERT_EQ(ctx.getErrors().size(), 0);
}

EVALUATOR_TEST(UndefinedIdentifier) {
    Context ctx(nullptr);
    ctx.phase = PassPhase::TYPE_CHECK;

    auto id = std::make_unique<IdentyfierNode>(PositionSpan(0,0), "y");
    std::vector<std::unique_ptr<ASTNode>> nodes;
    nodes.push_back(std::move(id));

    for (auto& node : nodes) {
        node->evaluateSymbol(ctx);
    }

    ASSERT_EQ(ctx.getErrors().size(), 1);
    ASSERT_TRUE(ctx.getErrors()[0].returnError().find("y") != std::string::npos);
}
