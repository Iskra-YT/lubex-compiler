#include "parser/ast.hpp"
#include "evaluator.hpp"
#include "parser/parser.hpp"
#include "debug.hpp"
#include <filesystem>
#include <ios>
#include <fstream>

extern IdentyfierNode intType;
extern IdentyfierNode objectType;
extern IdentyfierNode voidType;
extern IdentyfierNode stringType;
extern IdentyfierNode nullType;

extern std::filesystem::path mainSource;

static bool isTypeCompatible(Symbol* targetType, Symbol* valueType, std::string& errorMsg) {
    if (!targetType || !valueType) return true;

    std::string tName = targetType->name->value;
    std::string vName = valueType->name->value;

    // null -> T? : compatible
    if (vName == "Null") {
        if (targetType->isNullable) return true;
        errorMsg = "Cannot assign null to non-nullable type '" + tName + "'";
        return false;
    }

    // T? -> T : NOT compatible (must unwrap via ?? or ?:)
    if (valueType->isNullable && !targetType->isNullable) {
        errorMsg = "Cannot assign nullable type '" + vName + "?' to non-nullable type '" + tName + "'. Use '\?\?' to unwrap or '?:' to provide default";
        return false;
    }

    // Different base types
    if (tName != vName) {
        errorMsg = "Type mismatch: cannot assign '" + vName + "' to '" + tName + "'";
        return false;
    }

    return true;
}

bool inStatic = false;

Symbol* ASTNode::evaluateSymbol(Context& ctx) {
    throw std::runtime_error("Internal error: unreachable path");
}

Symbol* StatementNode::evaluateSymbol(Context& ctx) {
    return value->evaluateSymbol(ctx);
}

Symbol* NumberNode::evaluateSymbol(Context& ctx) {
    auto sym = ctx.lookup(&intType);
    sym->type = sym;
    return sym;
}

inline std::string getOperationFunction(std::string op) {
    if (op == "+") return "add";
    else if (op == "-") return "subtract";
    else if (op == "*") return "multiply";
    else if (op == "/") return "divide";
    else if (op == "==") return "equals";
    else if (op == "!=") return "notEquals";
    else if (op == "<") return "lessThan";
    else if (op == ">") return "greaterThan";
    else if (op == "<=") return "lessOrEqual";
    else if (op == ">=") return "greaterOrEqual";
    else if (op == "!") return "logicalNot";
    else if (op == "~") return "bitwiseNot";
    return "unknown";
}

Symbol* BinaryNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto L = left->evaluateSymbol(ctx);
        auto R = right->evaluateSymbol(ctx);

        if (L->type->name->value != R->type->name->value) {
            ctx.errors.push_back(Error(position, "Type mismatch in binary expression", mainSource.filename().string()));
        }

        auto getFunction = L->type->scope->lookup(getOperationFunction(op));
        if (!getFunction) {
            ctx.errors.emplace_back(position, "Type does not support operator '" + op + "'", mainSource.filename().string());
        }

        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
            return ctx.lookup(&intType);
        }

        return L;
    }
    return nullptr;
}

Symbol* UnaryNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto V = value->evaluateSymbol(ctx);

        if (V->type->name->value != "Number") {
            ctx.errors.push_back(Error(position, "Unary operator '" + op + "' requires Number operand", mainSource.filename().string()));
        }

        auto getFunction = V->type->scope->lookup(getOperationFunction(op));
        if (!getFunction) {
            ctx.errors.emplace_back(position, "Type does not support operator '" + op + "'", mainSource.filename().string());
        }

        return ctx.lookup(&intType);
    }
    return nullptr;
}

Symbol* VariableDeclarationNode::evaluateSymbol(Context& ctx) {
    auto declareVariable = [&]() {
        if (type) {
            auto t = type->evaluateSymbol(ctx);

            if (value && ctx.generativeSymbol->kind != SymbolKind::CLASS) {
                auto v = value->evaluateSymbol(ctx);
                auto sym = ctx.lookup(static_cast<IdentyfierNode*>(name.get()));
                if (v && t && sym) {
                    std::string compatErr;
                    if (!isTypeCompatible(t, v->type, compatErr)) {
                        ctx.errors.push_back(Error(position, compatErr, mainSource.filename().string()));
                        return;
                    }
                }

                ctx.declare(std::make_unique<Symbol>(SymbolKind::VARIABLE, dynamic_cast<IdentyfierNode*>(name.get()), t, static_cast<ASTNode*>(this)));
            } else if (value) {
                ctx.errors.push_back(Error(position, "Cannot initialize class member variables", mainSource.filename().string()));
                return;
            } else {
                ctx.declare(std::make_unique<Symbol>(SymbolKind::VARIABLE, dynamic_cast<IdentyfierNode*>(name.get()), t, static_cast<ASTNode*>(this)));
                return;
            }
        } else {
            if (value) {
                auto v = value->evaluateSymbol(ctx);
                if (!v || !v->type) {
                    ctx.errors.push_back(Error(position, "Cannot infer type from initializer", mainSource.filename().string()));
                    return;
                }

                ctx.declare(std::make_unique<Symbol>(SymbolKind::VARIABLE, dynamic_cast<IdentyfierNode*>(name.get()), v->type, static_cast<ASTNode*>(this)));
            } else {
                ctx.errors.push_back(Error(position, "Variable declaration must have a type or an initializer", mainSource.filename().string()));
                return;
            }
        }
    };

    if (ctx.generativeSymbol && ctx.generativeSymbol->kind == SymbolKind::CLASS && ctx.phase == PassPhase::DECLARATION) {
        declareVariable();
    } else if (ctx.phase == PassPhase::TYPE_CHECK && ctx.generativeSymbol && ctx.generativeSymbol->kind != SymbolKind::CLASS) {
        declareVariable();
    }

    return nullptr;
}

Symbol* IdentyfierNode::evaluateSymbol(Context& ctx) {
    return ctx.lookup(this);
}

Symbol* VariableAssigment::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto n = name->evaluateSymbol(ctx);
        auto v = value->evaluateSymbol(ctx);

        if (!n || !v) return nullptr;
        if (!n || n->kind != SymbolKind::VARIABLE) {
            ctx.errors.push_back(Error(position, "Cannot assign to non-variable symbol", mainSource.filename().string()));
        } else {
            std::string compatErr;
            if (!isTypeCompatible(n->type, v->type, compatErr)) {
                ctx.errors.push_back(Error(position, compatErr, mainSource.filename().string()));
            }
        } 
        
        if (dynamic_cast<VariableDeclarationNode*>(n->node) && dynamic_cast<VariableDeclarationNode*>(n->node)->isConst) {
            ctx.errors.push_back(Error(position, "Cannot assign to constant variable", mainSource.filename().string()));
        }
    }

    return nullptr;
}

Symbol* ArgDeclaration::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::DECLARATION) {
        ctx.declare(std::make_unique<Symbol>(
            SymbolKind::VARIABLE,
            static_cast<IdentyfierNode*>(name.get()),
            nullptr,
            static_cast<ASTNode*>(this)
        ));
    }

    if (ctx.phase == PassPhase::MIDPASS) {
        ctx.declare(std::make_unique<Symbol>(
            SymbolKind::VARIABLE,
            static_cast<IdentyfierNode*>(name.get()),
            type->evaluateSymbol(ctx),
            static_cast<ASTNode*>(this)
        ));
    }

    return nullptr;
}

Symbol* lookupWithInheritance(Symbol* cls, const std::string& name, Symbol* objectClass, bool isCall = false);

Symbol* FunctionDeclaration::evaluateSymbol(Context& ctx) {
    if (ctx.symbolKind != SymbolKind::CLASS) {
        ctx.errors.push_back(Error(position, "Function declarations are only allowed inside class declarations", mainSource.filename().string()));
        return nullptr;
    }

    if (body.size() == 0 && !isForward) {
        ctx.errors.push_back(Error(position, "Function body cannot be empty", mainSource.filename().string()));
        return nullptr;
    }

    if (ctx.phase == PassPhase::DECLARATION) {
        auto fnSym = std::make_unique<Symbol>(
            SymbolKind::FUNCTION,
            static_cast<IdentyfierNode*>(name.get()),
            nullptr,
            static_cast<ASTNode*>(this)
        );

        Context* fnCtx = ctx.addChild();
        fnCtx->symbolKind = SymbolKind::FUNCTION;
        fnCtx->phase = ctx.phase;

        fnSym->isStatic = isStatic ? true : false;

        fnSym->scope = fnCtx;
        fnSym->scope->generativeSymbol = fnSym.get();

        ctx.declare(std::move(fnSym));
    }

    if (ctx.phase == PassPhase::MIDPASS) {
        auto fnSym = ctx.lookup(static_cast<IdentyfierNode*>(name.get()));
        fnSym->type = type->evaluateSymbol(ctx);

        Context* fnCtx = fnSym->scope;
        fnCtx->phase = ctx.phase;

        for (auto& param : parameters) {
            param->evaluateSymbol(*fnCtx);
        }

        auto currentClass = ctx.generativeSymbol;

        if (!currentClass->classTypes.empty()) {
            auto parent = currentClass->classTypes[0];
            auto parentFn = lookupWithInheritance(parent, fnSym->name->value, ctx.lookup(&objectType));

            if (parentFn && parentFn->kind == SymbolKind::FUNCTION) {
                auto parentDecl = static_cast<FunctionDeclaration*>(parentFn->node);

                if (parentDecl->parameters.size() != parameters.size()) {
                    ctx.errors.push_back(Error(position, "Override signature mismatch", mainSource.filename().string()));
                }

                if (parentFn->type->name->value != fnSym->type->name->value) {
                    ctx.errors.push_back(Error(position, "Override return type mismatch", mainSource.filename().string()));
                }

                if (!isOverride) {
                    ctx.errors.push_back(Error(position, "Overriding function must be marked with 'override'", mainSource.filename().string()));
                }
            }
        }
    }

    if (ctx.phase == PassPhase::TYPE_CHECK) {
        auto fnSym = ctx.lookup(static_cast<IdentyfierNode*>(name.get()));
        Context* fnCtx = fnSym->scope;
        fnCtx->phase = ctx.phase;

        if (isStatic) {
            inStatic = true;
        }

        bool hasReturn = false;
        for (auto& stmt : body) {
            auto result = stmt->evaluateSymbol(*fnCtx);
        
            if (dynamic_cast<ReturnNode*>(dynamic_cast<StatementNode*>(stmt.get())->value.get())) {
                hasReturn = true;
            }
        }

        auto returnType = fnSym->type->name->value;
        if (returnType != "Void" && !hasReturn && !isForward) {
            ctx.errors.push_back(Error(position, "Missing return statement", mainSource.filename().string()));
        }
    }

    inStatic = false;
    return nullptr;
}

Symbol* ClassDeclNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::DECLARATION) {
        auto sym = std::make_unique<Symbol>(
            SymbolKind::CLASS,
            static_cast<IdentyfierNode*>(name.get()),
            ctx.lookup(&objectType),
            static_cast<ASTNode*>(this)
        );

        sym->scope = ctx.addChild();
        sym->scope->symbolKind = SymbolKind::CLASS;
        sym->scope->generativeSymbol = sym.get();
        ctx.declare(std::move(sym));
    }

    auto classSym = ctx.lookup(static_cast<IdentyfierNode*>(name.get()));
    auto classCtx = classSym->scope;

    if (ctx.phase == PassPhase::MIDPASS && parent) {
        classSym->classTypes.push_back(parent->evaluateSymbol(ctx));
    }

    classCtx->phase = ctx.phase;
    for (auto& member : members) {
        member->evaluateSymbol(*classCtx);
    }

    return nullptr;
}

Symbol* ModuleDeclaration::evaluateSymbol(Context& ctx) {
    if (ctx.phase == PassPhase::DECLARATION) {
        auto sym = std::make_unique<Symbol>(
            SymbolKind::MODULE,
            static_cast<IdentyfierNode*>(name.get()),
            nullptr,
            static_cast<ASTNode*>(this)
        );

        sym->scope = &ctx;
        ctx.generativeSymbol = sym.get();

        ctx.parent->declare(std::move(sym));
    }

    ctx.symbolKind = SymbolKind::MODULE;

    return nullptr;
}

Symbol* lookupWithInheritance(Symbol* cls, const std::string& name, Symbol* objectClass, bool isCall) {
    if (!cls || !cls->scope) return nullptr;

    if (auto sym = cls->scope->lookup(name)) {
        return sym;
    }

    for (auto& parent : cls->classTypes) {
        auto found = lookupWithInheritance(parent, name, objectClass, isCall);
        if (found) return found;
    }

    if (cls != objectClass) {
        return lookupWithInheritance(objectClass, name, objectClass, isCall);
    }

    return nullptr;
}

extern std::unordered_map<std::string, std::unordered_map<std::string, int>> numberOfParameters;

Symbol* CallNode::evaluateSymbol(Context& ctx) {
    auto call = callee->evaluateSymbol(ctx);
    if (!call) return nullptr;

    for (auto& arg : args) {
        arg->evaluateSymbol(ctx);
    }

    if (call->kind != SymbolKind::FUNCTION && call->kind != SymbolKind::CLASS) {
        ctx.errors.push_back(Error(position, "Called object is not a function", mainSource.filename().string()));
        return nullptr;
    }

    auto called = call;
    if (call->kind == SymbolKind::CLASS) {
        call = lookupWithInheritance(call, "init", ctx.lookup(&objectType), true);

        if (!call) {
            ctx.errors.push_back(Error(position, "Class has no constructor (init)", mainSource.filename().string()));
            return nullptr;
        }

        return called->createInstance();
    }

    if (called->name->value != "Int" && called->name->value != "Void") {
        if (call->node && static_cast<FunctionDeclaration*>(call->node)->parameters.size() > args.size()) { 
            ctx.errors.push_back(Error(position, "Argument count mismatch in function call", mainSource.filename().string()));
        } else if (!call->node && numberOfParameters[called->name->value][call->name->value] > args.size()) {
            ctx.errors.push_back(Error(position, "Argument count mismatch in function call", mainSource.filename().string()));
        }
    }
    
    return call;
}

Symbol* MemberAccessNode::evaluateSymbol(Context& ctx) {
    auto obj = object->evaluateSymbol(ctx);
    if (!obj) {
        return nullptr;
    }

    if (obj->type && obj->type->isNullable) {
        ctx.errors.push_back(Error(position, "Cannot use '.' on nullable type '" + obj->type->name->value + "?'. Use '?.' instead", mainSource.filename().string()));
        return nullptr;
    }

    if (obj->type && obj->type->name->value == "Null") {
        ctx.errors.push_back(Error(position, "Cannot use '.' on null literal", mainSource.filename().string()));
        return nullptr;
    }

    Symbol* memberSym = nullptr;
    bool isStaticAccess = false;

    if (obj->kind == SymbolKind::CLASS || obj->kind == SymbolKind::MODULE) {
        isStaticAccess = true;

        if (!obj->scope) {
            ctx.errors.push_back(Error(position, "Class or module has no scope", mainSource.filename().string()));
            return nullptr;
        }

        memberSym = lookupWithInheritance(obj, static_cast<IdentyfierNode*>(member.get())->value, ctx.lookup(&objectType));
    } else {
        if (!obj->type || obj->type->kind != SymbolKind::CLASS || !obj->type->scope) {
            ctx.errors.push_back(Error(position, "Object is not a class instance", mainSource.filename().string()));
            return nullptr;
        }

        memberSym = lookupWithInheritance(obj->type, static_cast<IdentyfierNode*>(member.get())->value, ctx.lookup(&objectType));
    }

    if (!memberSym) {
        ctx.errors.push_back(Error(position, "No such class member", mainSource.filename().string()));
        return nullptr;
    }

    if (memberSym->name->value == "init") {
        ctx.errors.push_back(Error(position, "Direct call to init() is not allowed", mainSource.filename().string()));
        return nullptr;
    }

    if (memberSym->kind == SymbolKind::FUNCTION) {
        auto funcDecl = static_cast<FunctionDeclaration*>(memberSym->node);

        if (isStaticAccess) {
            if (!memberSym->isStatic) {
                ctx.errors.push_back(Error(position, "Cannot access non-static member without instance", mainSource.filename().string()));
                return nullptr;
            }
        } else {
            if (memberSym->isStatic) {
                ctx.errors.push_back(Error(position, "Cannot access static member through instance", mainSource.filename().string()));
                return nullptr;
            }
        }

        if (funcDecl && funcDecl->visibility == VisibilityKind::PRIVATE) {
            if (!ctx.generativeSymbol ||
                obj->type->name->value != ctx.generativeSymbol->name->value) {
                ctx.errors.push_back(Error(position, "Cannot access private member from outside the class", mainSource.filename().string()));
                return nullptr;
            }
        }
    }

    return memberSym;
}

Symbol* ReturnNode::evaluateSymbol(Context& ctx) {
    if (value) {
        auto valueSym = value->evaluateSymbol(ctx);
        if (valueSym && valueSym->type && ctx.generativeSymbol) {
            std::string compatErr;
            if (!isTypeCompatible(ctx.generativeSymbol->type, valueSym->type, compatErr)) {
                ctx.errors.push_back(Error(position, "Type mismatch in return statement: " + compatErr, mainSource.filename().string()));
            }
        }

        return valueSym;
    } else {
        if (ctx.generativeSymbol->type->name->value != "Void") {
            ctx.errors.push_back(Error(position, "Type mismatch in return statement", mainSource.filename().string()));
        }

        auto voidSym = ctx.lookup(&voidType);
        voidSym->type = voidSym;
        return voidSym;
    }
}

Symbol* AttributesNode::evaluateSymbol(Context &ctx) {
    return value->evaluateSymbol(ctx);
}

#include "config.hpp"

extern bool compile(std::filesystem::path, Context&);
extern ProjectConfig config;
extern bool parsingModule;

Symbol* ImportNode::evaluateSymbol(Context& ctx){
    auto currentMs = mainSource;
    if (ctx.phase != PassPhase::DECLARATION) return nullptr;
    parsingModule = true;

    auto name = static_cast<IdentyfierNode*>(value.get());
    std::filesystem::path moduleName;
    if (name->value == "std") {
        moduleName = std::filesystem::path(config.options.stdPath);
    } else {
        moduleName = std::filesystem::path(config.sourceDir) / std::filesystem::path(name->value + ".lbx");
    }

    DEBUG_OUTPUT << moduleName << "\n";

    Context* moduleCtx = ctx.parent->addChild();
    moduleCtx->symbolKind = SymbolKind::NOT;

    if (!compile(moduleName, *moduleCtx)) {
        ctx.errors.emplace_back(position, "Cannot import module " + name->value, currentMs.filename().string());
    }

    parsingModule = false;
    mainSource = currentMs;
    return nullptr;
}

Symbol* StringNode::evaluateSymbol(Context& ctx) {
    auto sym = ctx.lookup(&stringType);
    sym->type = sym;
    return sym;
}

Symbol* ThisNode::evaluateSymbol(Context& ctx) {
    if (ctx.symbolKind != SymbolKind::FUNCTION) {
        ctx.errors.push_back(Error(position, "Cannot use 'this' outside of a function", mainSource.filename().string()));
        return nullptr;
    }


    if (inStatic) {
        ctx.errors.push_back(Error(position, "Cannot use 'this' in static context", mainSource.filename().string()));
        return nullptr;
    }

    DEBUG_OUTPUT << "ctx.parent->generativeSymbol: " << (ctx.parent && ctx.parent->generativeSymbol ? ctx.parent->generativeSymbol->name->value : "nullptr") << "\n";
    auto sym = ctx.parent->generativeSymbol->clone();
    sym->type = sym;
    return sym;
}

Symbol* NullNode::evaluateSymbol(Context& ctx) {
    auto sym = ctx.lookup(&nullType);
    sym->type = sym;
    return sym;
}

Symbol* NullableTypeNode::evaluateSymbol(Context& ctx) {
    auto baseTypeSym = baseType->evaluateSymbol(ctx);
    if (!baseTypeSym) return nullptr;

    auto* nullableSym = new Symbol(SymbolKind::CLASS, baseTypeSym->name, baseTypeSym, nullptr);
    nullableSym->isNullable = true;
    nullableSym->scope = baseTypeSym->scope;
    nullableSym->forcedMangle = baseTypeSym->forcedMangle;
    return nullableSym;
}

Symbol* NullCoalescingNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase != PassPhase::TYPE_CHECK) return nullptr;

    auto L = left->evaluateSymbol(ctx);
    auto R = right->evaluateSymbol(ctx);
    if (!L || !R || !L->type || !R->type) return nullptr;

    std::string leftBase = L->type->name->value;
    bool leftIsNullable = L->type->isNullable || leftBase == "Null";

    if (!leftIsNullable) {
        ctx.errors.push_back(Error(position, "Left operand of '?:' must be nullable type", mainSource.filename().string()));
        return nullptr;
    }

    if (leftBase == "Null") leftBase = R->type->name->value;

    if (R->type->name->value != leftBase) {
        ctx.errors.push_back(Error(position, "Type mismatch in '?:' expression", mainSource.filename().string()));
        return nullptr;
    }

    auto baseSym = ctx.lookup(leftBase, position);
    baseSym->type = baseSym;
    return baseSym;
}

Symbol* NullCheckNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase != PassPhase::TYPE_CHECK) return nullptr;

    auto v = value->evaluateSymbol(ctx);
    if (!v || !v->type) return nullptr;

    if (!v->type->isNullable && v->type->name->value != "Null") {
        ctx.errors.push_back(Error(position, "Operand of '\?\?' must be nullable type", mainSource.filename().string()));
        return nullptr;
    }

    if (v->type->name->value == "Null") {
        ctx.errors.push_back(Error(position, "Cannot use '\?\?' on null literal", mainSource.filename().string()));
        return nullptr;
    }

    auto baseSym = ctx.lookup(v->type->name->value, position);
    baseSym->type = baseSym;
    return baseSym;
}

Symbol* SafeNavigationNode::evaluateSymbol(Context& ctx) {
    if (ctx.phase != PassPhase::TYPE_CHECK) return nullptr;

    auto obj = object->evaluateSymbol(ctx);
    if (!obj) return nullptr;

    Symbol* objBaseType = obj->type;
    if (!objBaseType) return nullptr;

    if (!objBaseType->isNullable && objBaseType->name->value != "Null") {
        ctx.errors.push_back(Error(position, "Left-hand side of '?.' must be nullable type", mainSource.filename().string()));
        return nullptr;
    }

    if (objBaseType->name->value == "Null") {
        ctx.errors.push_back(Error(position, "Cannot use '?.' on null literal", mainSource.filename().string()));
        return nullptr;
    }

    Symbol* classToSearch = objBaseType;
    std::string memberName = static_cast<IdentyfierNode*>(member.get())->value;

    Symbol* memberSym = lookupWithInheritance(classToSearch, memberName, ctx.lookup(&objectType));
    if (!memberSym) {
        ctx.errors.push_back(Error(position, "Type '" + objBaseType->name->value + "' has no member '" + memberName + "'", mainSource.filename().string()));
        return nullptr;
    }

    if (!memberSym->type) {
        ctx.errors.push_back(Error(position, "Member has no type", mainSource.filename().string()));
        return nullptr;
    }

    auto resultType = ctx.lookup(memberSym->type->name->value, position);
    resultType->isNullable = true;
    resultType->type = resultType;
    return resultType;
}