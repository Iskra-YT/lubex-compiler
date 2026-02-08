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

    // We need to add Int class to nodes
    auto intClassNode = std::make_unique<StatementNode>(
        PositionSpan(0, 0),
        std::make_unique<ClassDeclNode>(
            PositionSpan(0, 0),
            std::make_unique<IdentyfierNode>(PositionSpan(0, 0), "Int"),
            std::vector<std::unique_ptr<ASTNode>>{},
            false
        )
    );

    nodes.push_back(std::move(intClassNode));

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

    auto lir = generateLIR(std::move(nodes), globalCtx);
    std::cout << "Size: " << lir.size() << "\n";
    for (auto& instr : lir) {
        instr->debug();
    }

    setEmiter(moduleName);

    // TEMP START:
    auto *intTy = llvm::Type::getInt32Ty(*emiterContext);
    auto *funcType = llvm::FunctionType::get(intTy, false);
    auto *mainFunc = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "main", emiterModule.get());

    auto *entry = llvm::BasicBlock::Create(*emiterContext, "entry", mainFunc);
    emiterBuilder->SetInsertPoint(entry);
    // TEMP END

    // TEMP START:
    emiterBuilder->CreateRet(llvm::ConstantInt::get(intTy, 0));
    // emiterModule->print(llvm::outs(), nullptr);
    // TEMP END:

    return true;
}