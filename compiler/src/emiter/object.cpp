#include "emiter/object.hpp"
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>

bool emitObjectFile(llvm::Module& module, const std::string& triple, const std::filesystem::path& output, ProjectConfig config) {
    std::string error;

    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple, error);
    if (!target) {
        llvm::errs() << "Target lookup failed: " << error << "\n";
        return false;
    }

    llvm::TargetOptions opt;

    llvm::Reloc::Model RM = llvm::Reloc::PIC_;
    auto targetMachine = std::unique_ptr<llvm::TargetMachine>(
        target->createTargetMachine(triple, "generic", "", opt, RM)
    );

    if (!targetMachine) {
        llvm::errs() << "Failed to create TargetMachine\n";
        return false;
    }

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
        default:
            targetMachine->setOptLevel(llvm::CodeGenOpt::Default);
            break;
    }

    module.setTargetTriple(triple);
    module.setDataLayout(targetMachine->createDataLayout());

    std::error_code EC;
    llvm::raw_fd_ostream dest(output.string(), EC, llvm::sys::fs::OF_None);

    if (EC) {
        llvm::errs() << "Could not open output file: " << EC.message() << "\n";
        return false;
    }

    llvm::legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile)) {
        llvm::errs() << "TargetMachine can't emit object file\n";
        return false;
    }

    pass.run(module);
    dest.flush();

    return true;
}