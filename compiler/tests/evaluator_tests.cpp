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

EVALUATOR_TEST(NestedContextLookup) {
    Context global(nullptr);
    auto sym1 = std::make_unique<Symbol>(SymbolKind::VARIABLE, new IdentyfierNode(PositionSpan(0,0), "a"), nullptr, nullptr);
    global.declare(std::move(sym1));

    Context* child = global.addChild();
    auto sym2 = std::make_unique<Symbol>(SymbolKind::VARIABLE, new IdentyfierNode(PositionSpan(0,0), "b"), nullptr, nullptr);
    child->declare(std::move(sym2));

    IdentyfierNode idA(PositionSpan(0,0), "a");
    IdentyfierNode idB(PositionSpan(0,0), "b");
    IdentyfierNode idC(PositionSpan(0,0), "c");

    ASSERT_NE(child->lookup(&idA), nullptr);
    ASSERT_NE(child->lookup(&idB), nullptr);
    ASSERT_EQ(child->lookup(&idC), nullptr);
}

EVALUATOR_TEST(DuplicateDeclarationError) {
    Context ctx(nullptr);
    auto id1 = new IdentyfierNode(PositionSpan(0,0), "x");
    auto sym1 = std::make_unique<Symbol>(SymbolKind::VARIABLE, id1, nullptr, nullptr);
    ctx.declare(std::move(sym1));

    auto id2 = new IdentyfierNode(PositionSpan(0,0), "x");
    auto sym2 = std::make_unique<Symbol>(SymbolKind::VARIABLE, id2, nullptr, nullptr);
    ctx.declare(std::move(sym2));

    ASSERT_EQ(ctx.getErrors().size(), 1);
    ASSERT_TRUE(ctx.getErrors()[0].returnError().find("already defined") != std::string::npos);
}
