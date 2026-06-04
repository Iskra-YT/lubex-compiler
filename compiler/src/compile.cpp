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
#include "emiter/emiter.hpp"
#include "emiter/object.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "optimizer.hpp"
#include "evaluator.hpp"
#include "parser/parser.hpp"
#include "LIR/lir.hpp"
#include "debug.hpp"

std::unordered_map<std::string, std::unordered_map<std::string, int>> numberOfParameters;

extern IdentyfierNode intType;
extern IdentyfierNode objectType;
extern IdentyfierNode voidType;
extern IdentyfierNode stringType;

IdentyfierNode initName(PositionSpan(0, 0), "init");
IdentyfierNode addName(PositionSpan(0, 0), "add");
IdentyfierNode subName(PositionSpan(0, 0), "subtract");
IdentyfierNode mulName(PositionSpan(0, 0), "multiply");
IdentyfierNode divName(PositionSpan(0, 0), "divide");
IdentyfierNode toStringName(PositionSpan(0, 0), "toString");

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

std::filesystem::path mainSource;

bool compile(std::filesystem::path ms, Context& globalCtx);

Symbol* createBuiltinClass(Context& globalCtx, std::unique_ptr<Context>& ctx, const std::string& name, IdentyfierNode* typeNode, const std::string& mangle) {
    auto cls = std::make_unique<Symbol>(SymbolKind::CLASS, typeNode, nullptr, nullptr);
    cls->forcedMangle = mangle;

    cls->scope = ctx.get();
    globalCtx.declare(std::move(cls));

    return globalCtx.lookup(typeNode);
}

Symbol* addBuiltinFunction(Context& ctx, Symbol* owner, IdentyfierNode* name, const std::string& mangle, bool isStatic = true) {
    auto fn = std::make_unique<Symbol>(SymbolKind::FUNCTION, name, owner, nullptr);
    fn->isStatic = isStatic;
    fn->forcedMangle = mangle;

    Symbol* raw = fn.get();
    ctx.declare(std::move(fn));
    return raw;
}

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
    auto stringContext = std::make_unique<Context>(&globalCtx);

    // Object
    auto object = std::make_unique<Symbol>(SymbolKind::CLASS, &objectType, nullptr, nullptr);
    object->forcedMangle = "_BI_Object";
    //Object.init
    auto func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, object.get(), nullptr);
    func->isStatic = true;
    func->forcedMangle = "_BI_Object_init";
    objectContext->declare(std::move(func));
    numberOfParameters["Object"]["init"] = 0;
    // Object.toString
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &toStringName, object.get(), nullptr);
    func->isStatic = false;
    func->forcedMangle = "_BI_Object_toString";
    objectContext->declare(std::move(func));
    numberOfParameters["Object"]["toString"] = 1;
    object->scope = objectContext.get();
    globalCtx.declare(std::move(object));

    // Number
    auto intClass = std::make_unique<Symbol>(SymbolKind::CLASS, &intType, nullptr, nullptr);
    // Number.init
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, intClass.get(), nullptr);
    func->isStatic = true;
     func->forcedMangle = "_BI_Number_init";
    numberOfParameters["Number"]["init"] = 1;
    intContext->declare(std::move(func));

    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &addName, intClass.get(), nullptr));
    numberOfParameters["Number"]["add"] = 2;
    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &subName, intClass.get(), nullptr));
    numberOfParameters["Number"]["subtract"] = 2;
    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &mulName, intClass.get(), nullptr));
    numberOfParameters["Number"]["multiply"] = 2;
    intContext->declare(std::make_unique<Symbol>(SymbolKind::FUNCTION, &divName, intClass.get(), nullptr));
    numberOfParameters["Number"]["divide"] = 2;
    intClass->scope = intContext.get();
    globalCtx.declare(std::move(intClass));

    // Void
    auto voidClass = std::make_unique<Symbol>(SymbolKind::CLASS, &voidType, nullptr, nullptr);
    // Void.init
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, voidClass.get(), nullptr);
    func->isStatic = true;
    numberOfParameters["Void"]["init"] = 0;
    voidContext->declare(std::move(func));
    voidClass->scope = voidContext.get();
    globalCtx.declare(std::move(voidClass));

    // String
    auto stringClass = std::make_unique<Symbol>(SymbolKind::CLASS, &stringType, nullptr, nullptr);
    func = std::make_unique<Symbol>(SymbolKind::FUNCTION, &initName, stringClass.get(), nullptr);
    func->isStatic = true;
    numberOfParameters["Number"]["init"] = 1;
    stringContext->declare(std::move(func));
    stringClass->scope = stringContext.get();
    globalCtx.declare(std::move(stringClass));

    if (!compile(mainSource, *globalCtx.addChild()))
        return false;

    for (auto& target : config.targets) {
        llvm::LLVMContext rttiCtx;
        std::filesystem::path buildDir = std::filesystem::current_path() / config.buildDir / target.machine;

        auto rttiModule = generateRTTIModule(rttiCtx, globalTypeInfos, "__rtti");
        emitObjectFile(*rttiModule, targetToTriple(target.machine), buildDir / "rtti.o", config);

#ifdef DEBUG
        rttiModule->print(llvm::errs(), nullptr);
#endif // DEBUG
    }
    
    return true;
}

std::unordered_map<std::string, std::vector<std::unique_ptr<ASTNode>>> moduleAST;
void printModuleASTPtrs() {
    DEBUG_OUTPUT << "moduleAST pointers {\n";
    for (const auto& pair : moduleAST) {
        const std::string& key = pair.first;
        const auto* vecPtr = &pair.second;
        DEBUG_OUTPUT << "  key: " << key << ", vector ptr: " << vecPtr << "\n";
        for (auto& node : pair.second) {
            DEBUG_OUTPUT << "    nodeptr: " << node.get() << "\n";
        }
    }
    DEBUG_OUTPUT << "}\n";
}

bool compile(std::filesystem::path ms, Context& globalCtx) {
    mainSource = ms;
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
        std::cerr << Error(PositionSpan(1, 1), "Source file is empty", mainSource.filename().string()).returnError() << "\n";
        return false;
    }

    Lexer lexer(buffer);
    std::vector<Token> tokens = lexer.lex();
    for (auto token : tokens) {
        if (token.type == TokenType::ERR_TOKEN) {
            errors.push_back(Error(token.position, "Unexpected token: " + token.value, mainSource.filename().string()));
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
        std::cerr << Error(PositionSpan(1, 1), "First declaration must be a module declaration", mainSource.filename().string()).returnError() << "\n";
        return false;
    }

    
    for (const auto& node : nodes) {
        node->debug();
        DEBUG_OUTPUT << "\n";
    }

    DEBUG_OUTPUT << "\n\n";

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
    DEBUG_OUTPUT << "\n\n";

    std::vector<std::unique_ptr<IRValue>> lir;
    try {
        lir = generateLIR(moduleAST[moduleName], mainCtx);
    } catch (const LIRException& e) {
        std::cerr << e.error.returnError() << "\n";
        return false;
    }
    
    DEBUG_OUTPUT << "Size: " << lir.size() << "\n";
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

            llvm::StructType* biIntType = llvm.structTypes["_BI_Number"];
            llvm::Value* callValue = llvm.emiterBuilder.CreateCall(callee);

            llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm.emiterContext), 0);
            llvm::Value* returnPtr = llvm.emiterBuilder.CreateStructGEP(biIntType, callValue, 1);

            llvm::Value* returnValue = llvm.emiterBuilder.CreateLoad(llvm::Type::getDoubleTy(llvm.emiterContext), returnPtr);

            llvm::Value* intVal = llvm.emiterBuilder.CreateFPToSI(returnValue, intType);
            llvm.emiterBuilder.CreateRet(intVal);
        }

#ifdef DEBUG
        llvm.emiterModule->print(llvm::outs(), nullptr);
#endif // DEBUG

        std::string targetTriple;
        try {
            targetTriple = targetToTriple(target.machine);
        } catch (std::exception& e) {
            std::cerr << e.what() << "\n";
            return false;
        }

        llvm.emiterModule->setTargetTriple(targetTriple);

        std::filesystem::path buildDir = std::filesystem::current_path() / config.buildDir / target.machine;
        std::filesystem::create_directories(buildDir);

        std::filesystem::path outputFile = buildDir / (moduleName + ".o");

        if (!emitObjectFile(*llvm.emiterModule, targetTriple, outputFile, config)) {
            std::cerr << "Failed to emit object file for " << target.machine << "\n";
            return false;
        }
    }

    return true;
}