#include <filesystem>
#include <fstream>
#include <ios>
#include "config.hpp"
#include "emiter.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "optimizer.hpp"
#include "parser.hpp"

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

    setEmiter(config.name);

    // TEMP START:
    auto *intTy = llvm::Type::getInt32Ty(*emiterContext);
    auto *funcType = llvm::FunctionType::get(intTy, false);
    auto *mainFunc = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "main", emiterModule.get());

    auto *entry = llvm::BasicBlock::Create(*emiterContext, "entry", mainFunc);
    emiterBuilder->SetInsertPoint(entry);
    // TEMP END

    Lexer lexer(buffer);
    std::vector<Token> tokens = lexer.lex();
    for (auto token : tokens) {
        if (token.type == TokenType::ERR_TOKEN) {
            errors.push_back(Error(token.position, "Unexpected token: " + token.value));
        }
    }

    if (!errors.empty()) {
        for (auto error : errors) {
            std::cout << error.returnError() << "\n";
            return false;
        }
    }

    Parser parser(tokens);
    auto nodes = parser.parse();

    if (!parser.getErrors().empty()) {
        for (auto error : parser.getErrors()) {
            std::cout << error.returnError() << "\n";
            return false;
        }
    }

    if (config.optimalization > 0) {
        getOptimization(&nodes);
    }

    for (const auto& node : nodes) {
        node->evaluate();
        std::cout << "\n";
    }

    // TEMP START:
    emiterBuilder->CreateRet(llvm::ConstantInt::get(intTy, 0));
    emiterModule->print(llvm::outs(), nullptr);
    // TEMP END:

    return true;
}