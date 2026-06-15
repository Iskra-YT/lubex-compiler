#include "emiter/emiter.hpp"

LLVMGenerator::LLVMGenerator(const std::string& moduleName) : emiterBuilder(emiterContext) {
    emiterModule = std::make_unique<llvm::Module>(moduleName, emiterContext);
    emiterModule->setDataLayout("e-m:e-i64:64-f80:128-n8:16:32:64-S128");

    generateBuildInStruct("_BI_TypeInfo", {"i128", "_BI_TypeInfo", "i64**"}, "");

    generateBuildInStruct("_BI_Object", {"_BI_TypeInfo"});
    generateBuildInFunction("_BI_Object_init", "_BI_Object", {"_BI_Object"}, "_BI_Object");  
    generateBuildInFunction("_BI_Object_toString", "_BI_Object", {"_BI_Object"}, "_BI_String");  

    generateBuildInStruct("_BI_Number", {"double"});
    generateBuildInFunction("_BI_Number_init", "_BI_Number", {"_BI_Number", "double"}, "_BI_Number");
    generateBuildInFunction("_BI_Number_add", "_BI_Number", {"_BI_Number", "_BI_Number"}, "_BI_Number");
    generateBuildInFunction("_BI_Number_subtract", "_BI_Number", {"_BI_Number", "_BI_Number"}, "_BI_Number");
    generateBuildInFunction("_BI_Number_multiply", "_BI_Number", {"_BI_Number", "_BI_Number"}, "_BI_Number");
    generateBuildInFunction("_BI_Number_divide", "_BI_Number", {"_BI_Number", "_BI_Number"}, "_BI_Number");

    generateBuildInStruct("_BI_Void", {});
    generateBuildInFunction("_BI_Void_init", "_BI_Void", {"_BI_Void"}, "_BI_Void"); 

    generateBuildInStruct("_BI_String", {"i8*", "i64"});
    generateBuildInFunction("_BI_String_init", "_BI_String", {"_BI_String", "i8*"}, "_BI_String");

    generateBuildInFunction("_BI_malloc", "void*", {"i64"}, "");

    // Runtime panic function: __R_panic(message: i8*) -> never (void)
    generateBuildInFunction("__R_panic", "void", {"i8*"}, "");
}

llvm::Function* LLVMGenerator::generateBuildInFunction(std::string name, std::string returnType, std::vector<std::string> argTypes, std::string className) {
    std::vector<llvm::Type*> args;
    for (auto arg : argTypes) {
        args.push_back(mapLLVMType(arg));
    }

    llvm::FunctionType* funcType = llvm::FunctionType::get(mapLLVMType(returnType), args, false);
    llvm::Function* func = llvm::Function::Create(funcType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name, emiterModule.get());
    
    if (!className.empty()) {
        typeMethods[className].push_back(func);
        builtinMethodClass[func] = className;
    }

    return func;
}
