#include "emiter/emiter.hpp"

llvm::Type* LLVMGenerator::mapLLVMType(const std::string& type, bool getPointer) {
    if (type == "i32") return llvm::Type::getInt32Ty(emiterContext);
    if (type == "double") return llvm::Type::getDoubleTy(emiterContext);
    if (type == "i1") return llvm::Type::getInt1Ty(emiterContext);
    if (type == "i64") return llvm::Type::getInt64Ty(emiterContext);
    if (type == "i64**") return llvm::Type::getInt64Ty(emiterContext)->getPointerTo()->getPointerTo();
    if (type == "i128") return llvm::Type::getInt128Ty(emiterContext);
    if (type == "void*") return llvm::Type::getVoidTy(emiterContext)->getPointerTo();
    if (type == "void**") return llvm::Type::getVoidTy(emiterContext)->getPointerTo()->getPointerTo();
    if (type == "i8**") return llvm::Type::getInt8Ty(emiterContext)->getPointerTo()->getPointerTo();

    auto it = structTypes.find(type);
    llvm::StructType* st = nullptr;
    if (it != structTypes.end()) {
        st = it->second;
    } else {
        st = llvm::StructType::create(emiterContext, type);
        structTypes[type] = st;
    }

    if (getPointer) {
        return st->getPointerTo();
    }

    return st;
}
