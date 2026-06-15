#include <gtest/gtest.h>
#include "LIR/lir.hpp"
#include "parser/parser.hpp"
#include "lexer.hpp"
#include "evaluator.hpp"
#include "lir_tests.hpp"
#include <filesystem>

extern std::filesystem::path mainSource;

// Helper to run the pipeline
std::vector<std::unique_ptr<IRValue>> runLIRPipeline(const std::string& source, Context& globalCtx) {
    mainSource = "test.lbx";
    std::vector<char> buffer(source.begin(), source.end());
    Lexer lexer(buffer);
    std::vector<Token> tokens = lexer.lex();
    
    Parser parser(tokens);
    auto nodes = parser.parse();
    
    if (!parser.getErrors().empty()) {
        throw std::runtime_error("Parser error: " + parser.getErrors()[0].returnError());
    }

    std::string moduleName = "test";
    if (!nodes.empty()) {
        if (auto stmt = dynamic_cast<StatementNode*>(nodes[0].get())) {
            if (auto mod = dynamic_cast<ModuleDeclaration*>(stmt->value.get())) {
                moduleName = static_cast<IdentyfierNode*>(mod->name.get())->value;
            }
        }
    }

    for (auto phase : {
        PassPhase::DECLARATION,
        PassPhase::MIDPASS,
        PassPhase::TYPE_CHECK
    }) {
        globalCtx.phase = phase;
        for (auto& node : nodes) {
            node->evaluateSymbol(globalCtx);
        }
        if (!globalCtx.getErrors().empty()) {
            throw std::runtime_error("Evaluator error in phase " + std::to_string((int)phase) + ": " + globalCtx.getErrors()[0].returnError());
        }
    }

    auto mainCtx = globalCtx.parent;
    normalizeSymbols(globalCtx, moduleName);

    return generateLIR(nodes, mainCtx);
}

// Built-in types nodes (declared in ast.cpp)
extern IdentyfierNode intType;
extern IdentyfierNode objectType;
extern IdentyfierNode voidType;
extern IdentyfierNode stringType;

extern IdentyfierNode initName;
extern IdentyfierNode toStringName;
extern IdentyfierNode addName;
extern IdentyfierNode subName;
extern IdentyfierNode mulName;
extern IdentyfierNode divName;

void setupGlobalContext(Context& globalCtx) {
    globalCtx.symbolKind = SymbolKind::NOT;

    auto objectContext = globalCtx.addChild();
    auto intContext = globalCtx.addChild();
    auto voidContext = globalCtx.addChild();
    auto stringContext = globalCtx.addChild();

    // Object
    auto object = std::make_unique<Symbol>(SymbolKind::CLASS, &objectType, nullptr, nullptr);
    object->forcedMangle = "_BI_Object";
    auto func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, object.get(), nullptr);
    func->isStatic = true;
    func->forcedMangle = "_BI_Object_init";
    objectContext->declare(std::move(func));
    object->scope = objectContext;
    globalCtx.declare(std::move(object));

    // Number
    auto intClass = std::make_unique<Symbol>(SymbolKind::CLASS, &intType, nullptr, nullptr);
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, intClass.get(), nullptr);
    func->isStatic = true;
    func->forcedMangle = "_BI_Number_init";
    intContext->declare(std::move(func));
    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &addName, intClass.get(), nullptr));
    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &subName, intClass.get(), nullptr));
    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &mulName, intClass.get(), nullptr));
    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &divName, intClass.get(), nullptr));
    intClass->scope = intContext;
    globalCtx.declare(std::move(intClass));

    // Void
    auto voidClass = std::make_unique<Symbol>(SymbolKind::CLASS, &voidType, nullptr, nullptr);
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, voidClass.get(), nullptr);
    func->isStatic = true;
    voidContext->declare(std::move(func));
    voidClass->scope = voidContext;
    globalCtx.declare(std::move(voidClass));

    // String
    auto stringClass = std::make_unique<Symbol>(SymbolKind::CLASS, &stringType, nullptr, nullptr);
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, stringClass.get(), nullptr);
    func->isStatic = true;
    stringContext->declare(std::move(func));
    stringClass->scope = stringContext;
    globalCtx.declare(std::move(stringClass));
}

LIR_TEST(BasicNumber) {
    Context globalCtx(nullptr);
    setupGlobalContext(globalCtx);
    
    std::string source = "module test; let a: Number = 5;";
    auto lir = runLIRPipeline(source, *globalCtx.addChild());

    ASSERT_FALSE(lir.empty());
    
    // For "let a: Number = 5;", we expect:
    // 1. Alloca for 'a'
    // 2. Alloca for temporary Number (for literal 5)
    // 3. Number literal value
    // 4. Call to _BI_Number_init
    // 5. Store literal to 'a'
    
    bool foundAllocaA = false;
    bool foundNumber5 = false;
    bool foundStore = false;

    for (const auto& instr : lir) {
        if (auto alloca = dynamic_cast<IRAlloca*>(instr.get())) {
            if (alloca->type == "_BI_Number") foundAllocaA = true;
        }
        if (auto num = dynamic_cast<IRNumber*>(instr.get())) {
            if (num->number == 5.0) foundNumber5 = true;
        }
        if (auto store = dynamic_cast<IRStore*>(instr.get())) {
            foundStore = true;
        }
    }

    EXPECT_TRUE(foundAllocaA);
    EXPECT_TRUE(foundNumber5);
    EXPECT_TRUE(foundStore);
}

LIR_TEST(BinaryExpression) {
    Context globalCtx(nullptr);
    setupGlobalContext(globalCtx);
    
    std::string source = "module test; let a: Number = 5 + 10;";
    auto lir = runLIRPipeline(source, *globalCtx.addChild());

    ASSERT_FALSE(lir.empty());

    bool foundAddCall = false;
    for (const auto& instr : lir) {
        if (auto call = dynamic_cast<IRCall*>(instr.get())) {
            if (call->funcName == "_BI_Number_add") foundAddCall = true;
        }
    }

    EXPECT_TRUE(foundAddCall);
}

LIR_TEST(FunctionDeclaration) {
    Context globalCtx(nullptr);
    setupGlobalContext(globalCtx);
    
    std::string source = "module test; class Program { static func add(a: Number, b: Number): Number { return a + b; }; };";
    auto lir = runLIRPipeline(source, *globalCtx.addChild());

    ASSERT_FALSE(lir.empty());

    bool foundFunction = false;
    for (const auto& instr : lir) {
        if (auto func = dynamic_cast<IRFunction*>(instr.get())) {
            if (func->name == "test.Program.add") {
                foundFunction = true;
                EXPECT_EQ(func->args.size(), 2);
                EXPECT_EQ(func->returnType, "_BI_Number");
            }
        }
    }

    EXPECT_TRUE(foundFunction);
}
