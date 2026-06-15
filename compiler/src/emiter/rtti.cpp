#include "emiter/emiter.hpp"
#include <iostream>
#include <unordered_set>

llvm::GlobalVariable* LLVMGenerator::updateTypeInfoWithVTable(llvm::GlobalVariable* typeInfo, llvm::Constant* vtablePtr) {
    auto* ty = structTypes["_BI_TypeInfo"];

    llvm::Constant* oldInit = typeInfo->getInitializer();
    auto* oldStruct = llvm::dyn_cast<llvm::ConstantStruct>(oldInit);

    if (!oldStruct) {
        std::cerr << "Invalid TypeInfo initializer\n";
        return typeInfo;
    }

    std::vector<llvm::Constant*> newFields;

    for (unsigned i = 0; i < 2; i++) {
        newFields.push_back(oldStruct->getOperand(i));
    }

    newFields.push_back(vtablePtr);

    llvm::Constant* newInit = llvm::ConstantStruct::get(ty, newFields);

    typeInfo->setInitializer(newInit);
    return typeInfo;
}

llvm::GlobalVariable* LLVMGenerator::getOrCreateTypeInfo(const std::string& name, const std::string& parentName) {
    if (typeInfos.count(name)) return typeInfos[name];

    Hash128 id;
    if (typeIds.count(name)) id = typeIds[name];
    else id = hash128(name);

    globalTypeInfos[name] = {name, parentName, id};

    auto* gv = new llvm::GlobalVariable(*emiterModule, structTypes["_BI_TypeInfo"], true, llvm::GlobalValue::ExternalLinkage, nullptr, "_T" + name);

    typeInfos[name] = gv;
    return gv;
}

void LLVMGenerator::buildVTable(const std::string& cls) {
    if (vTables.count(cls)) return;

    std::vector<llvm::Function*> finalMethods;
    std::unordered_map<std::string, int> indexMap;

    for (const auto& parent : classHierarchy[cls]) {
        if (parent.empty()) continue;

        buildVTable(parent);

        const auto& parentV = vTables[parent];
        for (auto* fn : parentV) {
            std::string name = getMethodName(fn->getName().str());

            if (!indexMap.count(name)) {
                indexMap[name] = static_cast<int>(finalMethods.size());
                finalMethods.push_back(fn);
            }
        }
    }

    for (auto* fn : typeMethods[cls]) {
        std::string name = getMethodName(fn->getName().str());

        if (indexMap.count(name)) {
            finalMethods[indexMap[name]] = fn;
        } else {
            indexMap[name] = static_cast<int>(finalMethods.size());
            finalMethods.push_back(fn);
        }
    }

    for (size_t i = 0; i < finalMethods.size(); i++) {
        std::string name = getMethodName(finalMethods[i]->getName().str());
        vTablePos[cls][name] = static_cast<int>(i);
    }

    auto& info = globalTypeInfos[cls];

    info.vtableName = "_VT" + cls;
    info.vtableMethods.clear();

    for (auto* fn : finalMethods) {
        info.vtableMethods.push_back(fn->getName().str());
    }

    auto* vtableType = llvm::ArrayType::get(llvm::Type::getInt8PtrTy(emiterContext), finalMethods.size());
    auto* vTableGlobal = new llvm::GlobalVariable(*emiterModule, vtableType, true, llvm::GlobalValue::ExternalLinkage, nullptr, "_VT" + cls);

    vTables[cls] = finalMethods;
}

std::unique_ptr<llvm::Module> generateRTTIModule(llvm::LLVMContext& ctx, const std::unordered_map<std::string, RTTIRecord>& globalTypeInfos, const std::string& moduleName) {
    auto module = std::make_unique<llvm::Module>(moduleName, ctx);
    auto* typeInfoTy = llvm::StructType::create(ctx, {llvm::Type::getInt128Ty(ctx), llvm::PointerType::getUnqual(ctx), llvm::Type::getInt8PtrTy(ctx)->getPointerTo()}, "_BI_TypeInfo");

    std::unordered_map<std::string, llvm::GlobalVariable*> typeInfos;
    std::unordered_set<std::string> declaredFunctions;

    for (auto& [_, info] : globalTypeInfos) {
        for (auto& fnName : info.vtableMethods) {
            if (declaredFunctions.contains(fnName)) continue;
            declaredFunctions.insert(fnName);

            auto* fnTy = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(ctx), true);
            llvm::Function::Create(
                fnTy,
                llvm::GlobalValue::ExternalLinkage,
                fnName,
                module.get()
            );
        }
    }

    for (auto& [name, _] : globalTypeInfos) {
        auto* gv = new llvm::GlobalVariable(*module, typeInfoTy, true, llvm::GlobalValue::ExternalLinkage, nullptr, "_T" + name);
        typeInfos[name] = gv;
    }

    std::unordered_map<std::string, llvm::GlobalVariable*> vtables;
    for (auto& [name, info] : globalTypeInfos) {
        std::vector<llvm::Constant*> entries;

        for (auto& fnName : info.vtableMethods) {
            auto* fn = module->getFunction(fnName);
            entries.push_back(llvm::ConstantExpr::getBitCast(fn, llvm::Type::getInt8PtrTy(ctx)));
        }

        auto* arrTy = llvm::ArrayType::get(llvm::Type::getInt8PtrTy(ctx), entries.size());
        auto* arr = llvm::ConstantArray::get(arrTy, entries);

        auto* gv = new llvm::GlobalVariable(*module, arrTy, true, llvm::GlobalValue::ExternalLinkage, arr, info.vtableName);
        vtables[name] = gv;
    }

    for (auto& [name, info] : globalTypeInfos) {

        llvm::Constant* parent = llvm::ConstantPointerNull::get(typeInfoTy->getPointerTo());

        if (!info.parent.empty()) {
            parent = typeInfos[info.parent];
        }

        llvm::Constant* idConst = llvm::ConstantInt::get(
            llvm::Type::getInt128Ty(ctx),
            toAPInt(info.id)
        );

        llvm::Constant* vtablePtr = llvm::ConstantExpr::getBitCast(
            vtables[name],
            llvm::Type::getInt8PtrTy(ctx)->getPointerTo()
        );

        auto* init = llvm::ConstantStruct::get(typeInfoTy,
            {
                idConst,
                parent,
                vtablePtr
            }
        );

        typeInfos[name]->setInitializer(init);
    }

    return module;
}
