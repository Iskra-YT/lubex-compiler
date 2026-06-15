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

EVALUATOR_TEST(ClassDeclarationViaEvaluator) {
    Context ctx(nullptr);
    ctx.phase = PassPhase::DECLARATION;

    auto numberIdent = IdentyfierNode(PositionSpan(0, 0), "Number");
    auto objectIdent = IdentyfierNode(PositionSpan(0, 0), "Object");
    ctx.declare(std::make_unique<Symbol>(SymbolKind::CLASS, &numberIdent, nullptr, static_cast<ASTNode*>(&numberIdent)));
    ctx.declare(std::make_unique<Symbol>(SymbolKind::CLASS, &objectIdent, nullptr, static_cast<ASTNode*>(&objectIdent)));

    auto className = std::make_unique<IdentyfierNode>(PositionSpan(0,0), "MyClass");
    auto classDecl = std::make_unique<ClassDeclNode>(
        PositionSpan(0,0),
        std::move(className),
        std::vector<std::unique_ptr<ASTNode>>(),
        false,
        VisibilityKind::PUBLIC,
        nullptr
    );

    classDecl->evaluateSymbol(ctx);
    ASSERT_EQ(ctx.getErrors().size(), 0);

    IdentyfierNode lookupName(PositionSpan(0,0), "MyClass");
    auto found = ctx.lookup(&lookupName);
    ASSERT_NE(found, nullptr);
    ASSERT_EQ(found->kind, SymbolKind::CLASS);
    ASSERT_NE(found->scope, nullptr);
}

EVALUATOR_TEST(NullableTypeResolution) {
    Context ctx(nullptr);
    ctx.phase = PassPhase::TYPE_CHECK;

    auto numberIdent = IdentyfierNode(PositionSpan(0, 0), "Number");
    ctx.declare(std::make_unique<Symbol>(SymbolKind::CLASS, &numberIdent, nullptr, static_cast<ASTNode*>(&numberIdent)));

    auto baseType = std::make_unique<IdentyfierNode>(PositionSpan(0,0), "Number");
    auto nullableType = std::make_unique<NullableTypeNode>(PositionSpan(0,0), std::move(baseType));

    auto sym = nullableType->evaluateSymbol(ctx);
    ASSERT_NE(sym, nullptr);
    ASSERT_TRUE(sym->isNullable);
    ASSERT_EQ(sym->name->value, "Number");
}

EVALUATOR_TEST(NullCoalescingTypeCheck) {
    Context ctx(nullptr);
    ctx.phase = PassPhase::TYPE_CHECK;

    auto numberIdent = IdentyfierNode(PositionSpan(0, 0), "Number");
    ctx.declare(std::make_unique<Symbol>(SymbolKind::CLASS, &numberIdent, nullptr, static_cast<ASTNode*>(&numberIdent)));

    auto leftIdent = std::make_unique<IdentyfierNode>(PositionSpan(0,0), "x");
    auto rightIdent = std::make_unique<IdentyfierNode>(PositionSpan(0,0), "y");

    auto coalescing = std::make_unique<NullCoalescingNode>(
        PositionSpan(0,0),
        std::move(leftIdent),
        std::move(rightIdent)
    );

    auto result = coalescing->evaluateSymbol(ctx);
    ASSERT_EQ(result, nullptr);
}

EVALUATOR_TEST(NullCheckOnNullable) {
    Context ctx(nullptr);
    ctx.phase = PassPhase::TYPE_CHECK;

    auto numberIdent = IdentyfierNode(PositionSpan(0, 0), "Number");
    ctx.declare(std::make_unique<Symbol>(SymbolKind::CLASS, &numberIdent, nullptr, static_cast<ASTNode*>(&numberIdent)));

    auto* numberSym = ctx.lookup(&numberIdent);

    auto nullType = std::make_unique<Symbol>(SymbolKind::CLASS, &numberIdent, numberSym, nullptr);
    nullType->isNullable = true;
    Symbol* nullTypePtr = nullType.get();

    auto varName = new IdentyfierNode(PositionSpan(0,0), "x");
    ctx.declare(std::make_unique<Symbol>(SymbolKind::VARIABLE, varName, nullTypePtr, nullptr));
    nullType.release();

    auto ident = std::make_unique<IdentyfierNode>(PositionSpan(0,0), "x");
    auto nullCheck = std::make_unique<NullCheckNode>(PositionSpan(0,0), std::move(ident));

    auto result = nullCheck->evaluateSymbol(ctx);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->name->value, "Number");
}

EVALUATOR_TEST(NullCheckOnNonNullable) {
    Context ctx(nullptr);
    ctx.phase = PassPhase::TYPE_CHECK;

    auto numberIdent = IdentyfierNode(PositionSpan(0, 0), "Number");
    ctx.declare(std::make_unique<Symbol>(SymbolKind::CLASS, &numberIdent, nullptr, static_cast<ASTNode*>(&numberIdent)));

    auto* numberSym = ctx.lookup(&numberIdent);

    auto varName = new IdentyfierNode(PositionSpan(0,0), "x");
    ctx.declare(std::make_unique<Symbol>(SymbolKind::VARIABLE, varName, numberSym, nullptr));

    auto ident = std::make_unique<IdentyfierNode>(PositionSpan(0,0), "x");
    auto nullCheck = std::make_unique<NullCheckNode>(PositionSpan(0,0), std::move(ident));

    auto result = nullCheck->evaluateSymbol(ctx);
    ASSERT_GT(ctx.getErrors().size(), 0);
    ASSERT_EQ(result, nullptr);
}

EVALUATOR_TEST(InheritedScopeLookup) {
    Context ctx(nullptr);
    ctx.phase = PassPhase::DECLARATION;

    auto numberIdent = IdentyfierNode(PositionSpan(0, 0), "Number");
    ctx.declare(std::make_unique<Symbol>(SymbolKind::CLASS, &numberIdent, nullptr, static_cast<ASTNode*>(&numberIdent)));

    auto parentCtx = ctx.addChild();
    auto parentName = new IdentyfierNode(PositionSpan(0,0), "parentVal");
    parentCtx->declare(std::make_unique<Symbol>(SymbolKind::VARIABLE, parentName, nullptr, nullptr));

    auto childCtx = parentCtx->addChild();
    auto childName = new IdentyfierNode(PositionSpan(0,0), "childVal");
    childCtx->declare(std::make_unique<Symbol>(SymbolKind::VARIABLE, childName, nullptr, nullptr));

    IdentyfierNode lookParent(PositionSpan(0,0), "parentVal");
    IdentyfierNode lookChild(PositionSpan(0,0), "childVal");
    IdentyfierNode lookMissing(PositionSpan(0,0), "missingVal");

    ASSERT_NE(childCtx->lookup(&lookParent), nullptr);
    ASSERT_NE(childCtx->lookup(&lookChild), nullptr);
    ASSERT_EQ(childCtx->lookup(&lookMissing), nullptr);
}
