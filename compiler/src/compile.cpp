#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
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

ProjectConfig config;
bool parsingModule = false;

std::string targetToTriple(const std::string& target) {
    std::unordered_map<std::string, std::string> arch_map = {
        {"x64", "x86_64"},
        {"x86", "i386"},
        {"arm64", "aarch64"},
        {"arm", "arm"}
    };

    std::unordered_map<std::string, std::string> os_map = {
        {"linux", "pc-linux-gnu"},
        {"windows", "pc-windows-msvc"},
        {"darwin", "apple-darwin"}
    };

    size_t dash_pos = target.find('-');
    if (dash_pos == std::string::npos) {
        throw std::invalid_argument("Target must be in format <os>-<arch>");
    }

    std::string os_part = target.substr(0, dash_pos);
    std::string arch_part = target.substr(dash_pos + 1);

    if (arch_map.find(arch_part) == arch_map.end()) {
        throw std::invalid_argument("Unknown architecture: " + arch_part);
    }

    if (os_map.find(os_part) == os_map.end()) {
        throw std::invalid_argument("Unknown OS: " + os_part);
    }

    return arch_map[arch_part] + '-' + os_map[os_part];
}

bool compile(std::filesystem::path mainSource, Context& globalCtx);

bool compileProject() {
    try {
        config = readConfig();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    std::filesystem::path mainSource = std::filesystem::current_path() / config.sourceDir / "main.lbx";
    Context globalCtx(nullptr);
    globalCtx.symbolKind = SymbolKind::NOT;

    auto objectContext = std::make_unique<Context>(&globalCtx);
    auto intContext = std::make_unique<Context>(&globalCtx);
    auto voidContext = std::make_unique<Context>(&globalCtx);

    // Object
    auto object = std::make_unique<Symbol>(SymbolKind::CLASS, &objectType, nullptr, nullptr);
    //Object.init
    auto func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, object.get(), nullptr);
    func->isStatic = true;
    objectContext->declare(std::move(func));

    objectContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &addName, object.get(), nullptr));
    objectContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &subName, object.get(), nullptr));
    objectContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &mulName, object.get(), nullptr));
    objectContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &divName, object.get(), nullptr));
    object->scope = objectContext.get();
    globalCtx.declare(std::move(object));

    // Int
    auto intClass = std::make_unique<Symbol>(SymbolKind::CLASS, &intType, nullptr, nullptr);
    // Int.init
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, intClass.get(), nullptr);
    func->isStatic = true;
    intContext->declare(std::move(func));

    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &addName, intClass.get(), nullptr));
    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &subName, intClass.get(), nullptr));
    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &mulName, intClass.get(), nullptr));
    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &divName, intClass.get(), nullptr));
    intClass->scope = intContext.get();
    globalCtx.declare(std::move(intClass));

    // Void
    auto voidClass = std::make_unique<Symbol>(SymbolKind::CLASS, &voidType, nullptr, nullptr);
    // Void.init
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, voidClass.get(), nullptr);
    func->isStatic = true;
    voidContext->declare(std::move(func));

    voidContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &addName, voidClass.get(), nullptr));
    voidContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &subName, voidClass.get(), nullptr));
    voidContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &mulName, voidClass.get(), nullptr));
    voidContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &divName, voidClass.get(), nullptr));
    voidClass->scope = voidContext.get();
    globalCtx.declare(std::move(voidClass));

    if (!compile(mainSource, *globalCtx.addChild()))
        return false;
    
    return true;
}

std::unordered_map<std::string, std::vector<std::unique_ptr<ASTNode>>> moduleAST;
void printModuleASTPtrs() {
    std::cout << "moduleAST pointers {" << std::endl;
    for (const auto& pair : moduleAST) {
        const std::string& key = pair.first;
        const auto* vecPtr = &pair.second;
        std::cout << "  key: " << key << ", vector ptr: " << vecPtr << std::endl;
        for (auto& node : pair.second) {
            std::cout << "    nodeptr: " << node.get() << std::endl;
        }
    }
    std::cout << "}" << std::endl;
}

bool compile(std::filesystem::path mainSource, Context& globalCtx) {
    std::ifstream mainSourceData(mainSource, std::ios::binary);
    if (!mainSourceData) {
        std::cerr << std::string("Error: Cannot open ") + std::string(mainSource) + std::string("\n");
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

    
    for (const auto& node : nodes) {
        node->debug();
        std::cout << "\n";
    }

    std::cout << "\n\n";

    std::string moduleName = static_cast<IdentyfierNode*>(static_cast<ModuleDeclaration*>(static_cast<StatementNode*>(nodes[0].get())->value.get())->name.get())->value;
    moduleAST[moduleName] = std::move(nodes);

    for (auto phase : {
        PassPhase::DECLARATION,
        PassPhase::MIDPASS,
        PassPhase::TYPE_CHECK
    }) {
        globalCtx.phase = phase;
        for (auto& node : moduleAST[moduleName]) {
            node->evaluateSymbol(globalCtx);
        }

        if (globalCtx.getErrors().size() != 0) {
            for (auto error : globalCtx.getErrors()) {
                std::cerr << error.returnError() << "\n";
                return false;
            }
        }
    }

    auto mainCtx = globalCtx.parent;
    normalizeSymbols(globalCtx, moduleName);
    printContext(mainCtx);
    std::cout << "\n\n";

    std::vector<std::unique_ptr<IRValue>> lir;
    try {
        lir = generateLIR(moduleAST[moduleName], mainCtx);
    } catch (const LIRException& e) {
        std::cerr << e.error.returnError() << "\n";
        return false;
    }
    
    std::cout << "Size: " << lir.size() << "\n";
    for (auto& instr : lir) {
        instr->debug();
    }

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();


    for (auto target : config.targets) {
        LLVMGenerator llvm(moduleName);
        llvm.generate(std::move(lir)); 

        if (!parsingModule) {
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
        }

        llvm.emiterModule->print(llvm::outs(), nullptr);

        std::string targetTriple;
        try {
            targetTriple = targetToTriple(target.machine);
        } catch (std::exception& e) {
            std::cerr << e.what() << "\n";
            return false;
        }

        llvm.emiterModule->setTargetTriple(targetTriple);

        std::string error;
        auto llvmTarget = llvm::TargetRegistry::lookupTarget(targetTriple, error);
        if (!llvmTarget) {
            llvm::errs() << "Target lookup failed for triple " << targetTriple << ": " << error << "\n";
            return false;
        }

        llvm::TargetOptions opt;
        auto RM = llvm::Optional<llvm::Reloc::Model>();
        auto targetMachine = llvmTarget->createTargetMachine(targetTriple, "generic", "", opt, RM);

        switch (config.optimalization) {
            case 0:
                targetMachine->setOptLevel(llvm::CodeGenOpt::None);
                break;
            case 1:
                targetMachine->setOptLevel(llvm::CodeGenOpt::Less);
                break;
            case 2:
                targetMachine->setOptLevel(llvm::CodeGenOpt::Default);
                break;
            case 3:
                targetMachine->setOptLevel(llvm::CodeGenOpt::Aggressive);
                break;
        }

        std::error_code EC;
        std::filesystem::path buildDir = std::filesystem::current_path() / config.buildDir / target.machine;
        std::filesystem::create_directories(buildDir);

        std::filesystem::path outputFile = buildDir / (moduleName + ".o");
        llvm::raw_fd_ostream dest(outputFile.string(), EC, llvm::sys::fs::OF_None);

        llvm::legacy::PassManager pass;

        if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile)) {
            llvm::errs() << "TargetMachine can't emit a file of this type";
            return false;
        }

        
        pass.run(*llvm.emiterModule.get());
        dest.flush();
    }

    return true;
}