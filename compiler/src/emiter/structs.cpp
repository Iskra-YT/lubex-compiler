#include "emiter/emiter.hpp"

llvm::StructType* LLVMGenerator::generateStruct(const std::string& name, const std::vector<llvm::Type*>& types, const std::string parentName) {
    std::vector<llvm::Type*> finalTypes;
    if (!parentName.empty() && structTypes.count(parentName)) {
        classHierarchy[name].push_back(parentName);
        auto* parentStruct = structTypes[parentName];
    
        for (auto* ty : parentStruct->elements()) {
            finalTypes.push_back(ty);
        }
    }

    for (auto* ty : types) {
        finalTypes.push_back(ty);
    }

    llvm::StructType* namedStruct = llvm::StructType::create(emiterContext, name);
    namedStruct->setBody(finalTypes);
    structTypes[name] = namedStruct;

    getOrCreateTypeInfo(name, parentName);
    return namedStruct;
}

llvm::StructType* LLVMGenerator::generateStruct(const std::string& name, const std::vector<llvm::Type*>& types, const std::vector<std::string>& parents) {
    std::vector<llvm::Type*> finalTypes;

    classHierarchy[name] = parents;

    for (auto& parentName : parents) {
        if (parentName.empty()) continue;
    
        auto it = structTypes.find(parentName);
        if (it == structTypes.end()) continue;
    
        auto* parentStruct = it->second;
    
        for (auto* ty : parentStruct->elements()) {
            finalTypes.push_back(ty);
        }
    }

    for (auto* ty : types) {
        finalTypes.push_back(ty);
    }

    llvm::StructType* namedStruct = llvm::StructType::create(emiterContext, name);
    namedStruct->setBody(finalTypes);
    structTypes[name] = namedStruct;

    getOrCreateTypeInfo(name, parents.back());

    return namedStruct;
}

llvm::StructType* LLVMGenerator::generateStruct(const std::string name, const std::vector<std::string> body, const std::string parentName) {
    std::vector<llvm::Type*> types;
    for (const auto& type : body) {
        types.push_back(mapLLVMType(type));
    }

    return generateStruct(name, types, parentName);
}

llvm::StructType* LLVMGenerator::generateBuildInStruct(std::string name, std::vector<std::string> body, const std::string parentName) {
    return generateStruct(name, body, parentName);
}
