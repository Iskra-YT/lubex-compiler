#include <filesystem>
#include <fstream>
#include <ios>
#include <unordered_set>
#include "config.hpp"
#include "emiter.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "optimizer.hpp"
#include "evaluator.hpp"
#include "parser/parser.hpp"
#include "LIR/lir.hpp"

extern IdentyfierNode intType;
extern IdentyfierNode objectType;
extern IdentyfierNode voidType;

IdentyfierNode initName(PositionSpan(0, 0), "init");
IdentyfierNode addName(PositionSpan(0, 0), "add");
IdentyfierNode subName(PositionSpan(0, 0), "subtract");
IdentyfierNode mulName(PositionSpan(0, 0), "multiply");
IdentyfierNode divName(PositionSpan(0, 0), "divide");

bool compileProject() {
    ProjectConfig config;
    try {
        config = readConfig();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    std::filesystem::path mainSource = std::filesystem::current_path() / config.sourceDir / "main.lbx";
    std::ifstream mainSourceData(mainSource, std::ios::binary);
    if (!mainSourceData) {
        std::cerr << "Error: Cannot open main.lbx\n";
        return EXIT_FAILURE;
    }

    mainSourceData.seekg(0, std::ios::end);
    size_t size = mainSourceData.tellg();
    mainSourceData.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    mainSourceData.read(buffer.data(), size);

    std::vector<Error> errors;

    if (size == 0) {
        std::cerr << Error(PositionSpan(1, 1), "Source file is empty").returnError() << "\n";
        return false;
    }

    Lexer lexer(buffer);
    std::vector<Token> tokens = lexer.lex();
    for (auto token : tokens) {
        if (token.type == TokenType::ERR_TOKEN) {
            errors.push_back(Error(token.position, "Unexpected token: " + token.value));
        }
    }

    if (!errors.empty()) {
        for (auto error : errors) {
            std::cerr << error.returnError() << "\n";
            return false;
        }
    }

    Parser parser(tokens);
    auto nodes = parser.parse();

    if (!parser.getErrors().empty()) {
        for (auto error : parser.getErrors()) {
            std::cerr << error.returnError() << "\n";
            return false;
        }
    }

    if (config.optimalization > 0) {
        getOptimization(&nodes);
    }

    if (!dynamic_cast<ModuleDeclaration*>(dynamic_cast<StatementNode*>(nodes[0].get())->value.get())) {
        std::cerr << Error(PositionSpan(1, 1), "First declaration must be a module declaration").returnError() << "\n";
        return false;
    }

    Context globalCtx(nullptr);
    globalCtx.symbolKind = SymbolKind::NOT;

    // Object
    Context objectContext;
    auto object = std::make_unique<Symbol>(SymbolKind::CLASS, &objectType, nullptr, nullptr);
    //Object.init
    auto func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, object.get(), nullptr);
    func->isStatic = true;
    objectContext.declare(std::move(func));

    objectContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &addName, object.get(), nullptr));
    objectContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &subName, object.get(), nullptr));
    objectContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &mulName, object.get(), nullptr));
    objectContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &divName, object.get(), nullptr));
    object->scope = &objectContext;
    globalCtx.declare(std::move(object));

    // Int
    Context intContext;
    auto intClass = std::make_unique<Symbol>(SymbolKind::CLASS, &intType, nullptr, nullptr);
    // Int.init
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, intClass.get(), nullptr);
    func->isStatic = true;
    intContext.declare(std::move(func));

    intContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &addName, intClass.get(), nullptr));
    intContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &subName, intClass.get(), nullptr));
    intContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &mulName, intClass.get(), nullptr));
    intContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &divName, intClass.get(), nullptr));
    intClass->scope = &intContext;
    globalCtx.declare(std::move(intClass));

    // Void
    Context voidContext;
    auto voidClass = std::make_unique<Symbol>(SymbolKind::CLASS, &voidType, nullptr, nullptr);
    // Void.init
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, voidClass.get(), nullptr);
    func->isStatic = true;
    voidContext.declare(std::move(func));

    voidContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &addName, voidClass.get(), nullptr));
    voidContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &subName, voidClass.get(), nullptr));
    voidContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &mulName, voidClass.get(), nullptr));
    voidContext.declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &divName, voidClass.get(), nullptr));
    voidClass->scope = &voidContext;
    globalCtx.declare(std::move(voidClass));

    for (auto phase : {
        PassPhase::DECLARATION,
        PassPhase::MIDPASS,
        PassPhase::TYPE_CHECK
    }) {
        globalCtx.phase = phase;
        for (auto& node : nodes) {
            node->evaluateSymbol(globalCtx);
        }
    }

    if (globalCtx.getErrors().size() != 0) {
        for (auto error : globalCtx.getErrors()) {
            std::cerr << error.returnError() << "\n";
            return false;
        }
    }

    for (const auto& node : nodes) {
        node->debug();
        std::cout << "\n";
    }

    std::string moduleName = static_cast<IdentyfierNode*>(static_cast<ModuleDeclaration*>(static_cast<StatementNode*>(nodes[0].get())->value.get())->name.get())->value;

    std::cout << "\n\n";
    normalizeSymbols(globalCtx, moduleName);
    printContext(&globalCtx, 0);
    std::cout << "\n\n";

    std::vector<std::unique_ptr<IRValue>> lir;
    try {
        lir = generateLIR(std::move(nodes), globalCtx);
    } catch (const LIRException& e) {
        std::cerr << e.error.returnError() << "\n";
        return false;
    }
    
    std::cout << "Size: " << lir.size() << "\n";
    for (auto& instr : lir) {
        instr->debug();
    }

    for (auto target : config.targets) {
        LLVMGenerator llvm(moduleName);
        llvm.generate(std::move(lir)); 

        llvm::Type* intType = llvm::Type::getInt32Ty(llvm.emiterContext);
        llvm::FunctionType* mainType = llvm::FunctionType::get(intType, false);
        llvm::Function* mainFunc = llvm::Function::Create(mainType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, "main", llvm.emiterModule.get());

        llvm::BasicBlock* entry = llvm::BasicBlock::Create(llvm.emiterContext, "entry", mainFunc);
        llvm.emiterBuilder.SetInsertPoint(entry);

        llvm::Function* callee = llvm.emiterModule->getFunction(mangleName(target.entrypoint));
        if (!callee) {
            std::cerr << "Function not found: " << target.entrypoint << "\n";
            break;
        }

        llvm::StructType* biIntType = llvm.structTypes["_BI_Int"];
        llvm::Value* callValue = llvm.emiterBuilder.CreateCall(callee);

        llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm.emiterContext), 0);
        llvm::Value* returnPtr = llvm.emiterBuilder.CreateGEP(biIntType, callValue, {zero, zero});

        llvm::Value* returnValue = llvm.emiterBuilder.CreateLoad(llvm::Type::getDoubleTy(llvm.emiterContext), returnPtr);

        llvm::Value* intVal = llvm.emiterBuilder.CreateFPToSI(returnValue, intType);
        llvm.emiterBuilder.CreateRet(intVal);

        llvm.emiterModule->print(llvm::outs(), nullptr);
    }

    return true;
}