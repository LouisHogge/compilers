#include "codeGeneration.hpp"
#include "ast.hpp"

CodeGeneration::CodeGeneration(const std::unordered_map<std::string, FirstPass::ClassInfo>& classInfoMap, const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classMethodTypes, const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classFieldTypes, const std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> classMethodFormalsTypes): classInfoMap_(classInfoMap), classMethodTypes_(classMethodTypes), classFieldTypes_(classFieldTypes), classMethodFormalsTypes_(classMethodFormalsTypes)
{
    auto& classNodeForFileName = classInfoMap_.at("Main").classNode;
    std::string currentFileName_ = classNodeForFileName->getFilename();

    llvmContext = std::make_unique<llvm::LLVMContext>();
    llvmModule = std::make_unique<llvm::Module>(currentFileName_, *llvmContext);
    llvmBuilder = std::make_unique<llvm::IRBuilder<>>(*llvmContext);

    declareStructuresAndVtables();
    declareMallocFunction();
    declarePowerFunction();
    declareObjectClassMethods();
    declareClassMethods();
    declareInheritedMethods();
    declareInheritedFields();

    // Create the 'main' function
    createLLVMFunction(getType("int32"), "main", {});
    llvm::Function* mainFunc = llvmModule->getFunction("main");

    // Set the insertion point to the start of 'main'
    auto mainBlock = llvm::BasicBlock::Create(*llvmContext, "entry", mainFunc);
    llvmBuilder->SetInsertPoint(mainBlock);

    // Call 'Main___new'
    llvm::Function* mainConstructor = llvmModule->getFunction("Main___new");
    if (!mainConstructor) {
        std::cerr << "Error" << std::endl;
    }
    llvm::Value* mainObj = llvmBuilder->CreateCall(mainConstructor);
    if (!mainObj) {
        std::cerr << "Error" << std::endl;
    }

    // Call 'Main__main'
    llvm::Function* mainMethod = llvmModule->getFunction("Main__main");
    if (!mainMethod) {
        std::cerr << "Error" << std::endl;
    }
    llvm::Value* mainResult = llvmBuilder->CreateCall(mainMethod, {mainObj});
    if (!mainResult) {
        std::cerr << "Error" << std::endl;
    }

    // Return the result of 'Main__main'
    llvmBuilder->CreateRet(mainResult);
}

// Function to create LLVM functions --> ok
void CodeGeneration::createLLVMFunction(llvm::Type* returnType, const std::string& functionName, const std::vector<llvm::Type*>& argTypes) {
    auto functionType = llvm::FunctionType::get(returnType, argTypes, false);

    llvm::Function* function = llvmModule->getFunction(functionName);

    if (!function) {
        function = llvm::Function::Create(functionType, llvm::GlobalValue::ExternalLinkage, functionName, llvmModule.get());
    }
}

llvm::Type* CodeGeneration::getInt32TyW(llvm::LLVMContext* context) {
    return llvm::Type::getInt32Ty(*context);
}

llvm::Type* CodeGeneration::getInt1TyW(llvm::LLVMContext* context) {
    return llvm::Type::getInt1Ty(*context);
}

llvm::Type* CodeGeneration::getInt8PtrTyW(llvm::LLVMContext* context) {
    return llvm::Type::getInt8PtrTy(*context);
}

llvm::Type* CodeGeneration::getType(const std::string type)
{
    std::unordered_map<std::string, llvm::Type* (CodeGeneration::*)(llvm::LLVMContext*)> type_map_ {
        {"int32", &CodeGeneration::getInt32TyW},
        {"bool", &CodeGeneration::getInt1TyW},
        {"string", &CodeGeneration::getInt8PtrTyW},
        {"unit", &CodeGeneration::getInt8PtrTyW}
    };

    auto found = type_map_.find(type);
    if (found != type_map_.end())
        return (this->*found->second)(llvmContext.get());
    else
        return llvm::PointerType::get(llvmModule->getTypeByName(type), 0);
}

void CodeGeneration::declareStructuresAndVtables()
{
    for (const auto& classIt : classInfoMap_) {
        const std::string& className = classIt.first;

        llvm::StructType::create(*llvmContext, className);
        
        llvm::StructType::create(*llvmContext, className + "__vtable");

    }
}

void CodeGeneration::declareMallocFunction()
{
    std::vector<llvm::Type *> argTypes;
    argTypes.push_back(llvm::Type::getInt64Ty(*llvmContext));

    createLLVMFunction(llvm::Type::getInt8PtrTy(*llvmContext), "malloc", argTypes);
}

void CodeGeneration::declarePowerFunction()
{
    std::vector<llvm::Type *> argTypes;
    argTypes.push_back(llvm::Type::getDoubleTy(*llvmContext));
    argTypes.push_back(llvm::Type::getDoubleTy(*llvmContext));

    createLLVMFunction(llvm::Type::getDoubleTy(*llvmContext), "pow", argTypes);
}

void CodeGeneration::declareObjectClassMethods()
{
    // ___new
    createLLVMFunction(getType("Object"), "Object___new", {});

    // ___init
    createLLVMFunction(getType("Object"), "Object___init", {getType("Object")});

    // Object
    auto objectClassMethodsIt = classMethodTypes_.find("Object");
    if (objectClassMethodsIt != classMethodTypes_.end()) {

        for (const auto& it : objectClassMethodsIt->second) {
            auto& methodName = it.first;
            auto& methodType = it.second;

            auto formals = std::vector<llvm::Type*>();
            formals.push_back(getType("Object"));

            auto classIt =  classMethodFormalsTypes_.find("Object");
            if (classIt != classMethodFormalsTypes_.end()) {

                auto methodIt = classIt->second.find(methodName);
                if (methodIt != classIt->second.end()) {

                    for (const auto& formalIt : methodIt->second) {
                        // auto& formalName = formalIt.first;
                        auto& formalType = formalIt.second;

                        formals.push_back(getType(formalType));
                    }
                }
            }

            createLLVMFunction(getType(methodType), "Object__" + methodName, formals);
        }
    } else {
        std::cerr << "Error: Object class not found." << std::endl;
    }
}

void CodeGeneration::declareClassMethods()
{
    for (const auto& classIt : classMethodTypes_) {
        const std::string& className = classIt.first;

        if (className == "Object") {
            continue;
        }

        // ___new
        createLLVMFunction(getType(className), className + "___new", {});

        // ___init
        createLLVMFunction(getType(className), className + "___init", {getType(className)});

        for (const auto& it : classMethodTypes_.find(className)->second) {
            auto& methodName = it.first;
            auto& methodType = it.second;

            auto formals = std::vector<llvm::Type*>();
            formals.push_back(getType(className));

            auto classIt =  classMethodFormalsTypes_.find(className);
            if (classIt != classMethodFormalsTypes_.end()) {

                auto methodIt = classIt->second.find(methodName);
                if (methodIt != classIt->second.end()) {

                    for (const auto& formalIt : methodIt->second) {
                        // auto& formalName = formalIt.first;
                        auto& formalType = formalIt.second;

                        formals.push_back(getType(formalType));
                    }
                }
            }
            
            createLLVMFunction(getType(methodType), className + "__" + methodName, formals);
        }
    }
}

// verify if order of methods and types has importance
void CodeGeneration::declareInheritedMethods()
{
    for (const auto& classIt : classMethodTypes_) {
        std::string currentChildClass = classIt.first;
        std::string currentClass = classInfoMap_.at(currentClass).parentClassName;

        if (currentClass == "Object") {
            continue;
        }

        llvm::StructType *structType = llvmModule->getTypeByName(currentChildClass + "__VTable");

        std::map<std::string, llvm::Constant*> llvmMethods;
        std::map<std::string, llvm::Type*> llvmTypes;

        // loop walks up class hierarchy
        while (classMethodTypes_.find(currentClass) != classMethodTypes_.end()) {

            for (const auto& methodIt : classMethodTypes_.find(currentClass)->second) {
                const std::string& methodName = methodIt.first;

                auto llvmFunction = llvmModule->getFunction(currentClass + "__" + methodName);
                auto llvmTypeFunction = llvmFunction->getFunctionType();
                auto llvmTypePointer = llvm::PointerType::get(llvmTypeFunction, 0);

                // update
                llvmMethods[methodName] = llvmFunction;
                llvmTypes[methodName] = llvmTypePointer;
            }
            currentClass = classInfoMap_.at(currentClass).parentClassName;
        }

        // Transform maps into vectors
        std::vector<llvm::Type*> llvmTypesV;
        for (const auto& item : llvmTypes) {
            llvmTypesV.push_back(item.second);
        }

        std::vector<llvm::Constant*> llvmMethodsV;
        for (const auto& item : llvmMethods) {
            llvmMethodsV.push_back(item.second);
        }

        structType->setBody(llvmTypesV);

        // ___vtable
        auto *structTypeBis = llvm::ConstantStruct::get(structType, llvmMethodsV);
        llvm::GlobalVariable *vtable = (llvm::GlobalVariable *) llvmModule->getOrInsertGlobal(currentChildClass + "___vtable", structType);
        vtable->setInitializer(structTypeBis);
        vtable->setConstant(true);
    }
}

void CodeGeneration::declareInheritedFields()
{
    for (const auto& classIt : classFieldTypes_) {
        const std::string& currentChildClass = classIt.first;
        const std::string& currentClass = classInfoMap_.at(currentChildClass).parentClassName;

        if (currentClass == "Object") {
            continue;
        }

        llvm::StructType *structType = (llvm::StructType *) llvmModule->getTypeByName(currentChildClass);

        auto llvmTypes = std::vector<llvm::Type*>();

        // __vtable
        auto llvmTypeName = llvmModule->getTypeByName(currentChildClass + "__vtable");
        auto llvmTypePointer = llvm::PointerType::get(llvmTypeName, 0);

        llvmTypes.push_back(llvmTypePointer);

        // loop walks up class hierarchy
        while (classFieldTypes_.find(currentClass) != classFieldTypes_.end()) {

            for (const auto& fieldIt : classFieldTypes_.find(currentClass)->second) {
                // const std::string& fieldName = fieldIt.first;
                const std::string& fieldType = fieldIt.second;

                llvmTypes.push_back(getType(fieldType));
            }
        }

        structType->setBody(llvmTypes);
    }
}

void CodeGeneration::displayIROnStdout()
{
    llvm::raw_os_ostream os(std::cout);
    llvmModule->print(os, nullptr);
}

void CodeGeneration::generateExecutable()
{
    std::string execName = currentFileName_.substr(0, currentFileName_.find_last_of("."));
    std::ofstream exec(execName + ".ll");
    if(!exec) {
        std::cerr << "Error: Failed to open output file." << std::endl;
    }

    llvm::raw_os_ostream os(exec);
    llvmModule->print(os, nullptr);

    // Close output file
    exec.close();

    // Execute command
    std::string cmd = "clang -Wno-override-llvmModule -o " + execName + " " + execName + ".ll" + " /usr/local/lib/vsop/*.c";
    int ret = system(cmd.c_str());
    if(ret != 0) {
        std::cerr << "Error: Failed to execute clang command." << std::endl;
    }
}

void CodeGeneration::cleanup() {
    llvmModule = nullptr;
}

// ProgramNode --> ClassNode
void CodeGeneration::visit(AST::ProgramNode* node) {
    currentProgramNode_ = node;

    for (auto& classNode : node->getClasses()) {
        classNode->accept(this);
    }

    generateExecutable();
    cleanup();
}

// ClassNode --> ClassBodyNode
void CodeGeneration::visit(AST::ClassNode* node) {
    currentClassName_ = node->getName();

    AST::ClassBodyNode* classBodyNode = node->getClassBody();
    if (classBodyNode != NULL) {
        classBodyNode->accept(this);
    }
}

// ClassBodyNode --> MethodNode and FieldNode
void CodeGeneration::visit(AST::ClassBodyNode* node) {
    // newFunction, ___new
    llvm::Function *newFunction = llvmModule->getFunction(currentClassName_ + std::string("___new"));
    auto newBlock = llvm::BasicBlock::Create(*llvmContext, "entry", newFunction);
    llvmBuilder->SetInsertPoint(newBlock);

    // malloc
    auto mallocFunction = llvmModule->getFunction("malloc");
    auto mallocData = std::make_unique<llvm::DataLayout>(llvmModule.get());
    auto mallocSize = mallocData->getTypeAllocSizeInBits(llvmModule->getTypeByName(currentClassName_));
    std::vector<llvm::Value*> mallocArgs = {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*(llvmContext)), mallocSize)};
    auto mallocCall = llvmBuilder->CreateCall(mallocFunction, mallocArgs);

    // parentClass
    std::string parentClassName = classInfoMap_.at(currentClassName_).parentClassName;
    auto parentClassPointerCast = llvmBuilder->CreatePointerCast(mallocCall, llvm::PointerType::get(llvmModule->getTypeByName(parentClassName), 0));
    // ___init
    auto parentClassInit = llvmModule->getFunction(parentClassName + std::string("___init"));
    parentClassPointerCast = llvmBuilder->CreateCall(parentClassInit, {parentClassPointerCast});

    // childClass
    auto childClassPointerCast = llvmBuilder->CreatePointerCast(parentClassPointerCast, llvm::PointerType::get(llvmModule->getTypeByName(currentClassName_), 0));
    // ___init
    auto childClassInit = llvmModule->getFunction(currentClassName_ + std::string("___init"));
    childClassPointerCast = llvmBuilder->CreateCall(childClassInit, {childClassPointerCast});
    llvmBuilder->CreateRet(childClassPointerCast);

    // initFunction
    auto initFunction = llvmModule->getFunction(currentClassName_ + std::string("___init"));
    auto initBlock = llvm::BasicBlock::Create(*(llvmContext), "entry", initFunction);
    llvmBuilder->SetInsertPoint(initBlock);

    // ___vtable
    auto thisPtr = initFunction->args().begin();
    auto vtable = llvmBuilder->CreateStructGEP(llvmModule->getTypeByName(currentClassName_), thisPtr, 0);
    llvmBuilder->CreateStore(llvmModule->getNamedGlobal(currentClassName_ + "___vtable"), vtable);

    for (auto& fieldNode : node->getFields()) {
        fieldNode->accept(this);
    }

    llvmBuilder->CreateRet(thisPtr);

    for (auto& methodNode : node->getMethods()) {
        methodNode->accept(this);
    }
}

// FieldNode --> TypeNode and ExprNode
void CodeGeneration::visit(AST::FieldNode* node) {
    std::string currentClass = classInfoMap_.at(currentClassName_).parentClassName;
    std::string fieldName = node->getName();
    AST::TypeNode* fieldTypeNode = node->getType();
    const std::string fieldType = fieldTypeNode->getTypeStr();

    // loop walks up class hierarchy
    while (classFieldTypes_.find(currentClass) != classFieldTypes_.end()) {
        if (classFieldTypes_.at(currentClass).find(fieldName) != classFieldTypes_.at(currentClass).end()) {

            for (const auto& fieldIt : classFieldTypes_.find(currentClass)->second) {
                const std::string& fieldName = fieldIt.first;
                const std::string& fieldType = fieldIt.second;

                // ...
            }
        }
        currentClass = classInfoMap_.at(currentClass).parentClassName;
    }
}

// unused
void CodeGeneration::visit(AST::TypeNode* /*node*/) {}
void CodeGeneration::visit(AST::FormalNode* /*node*/) {}
void CodeGeneration::visit(AST::FormalsNode* /*node*/) {}
void CodeGeneration::visit(AST::ExprNode* /*node*/) {}
void CodeGeneration::visit(AST::IfThenElseExprNode* /*node*/) {}
void CodeGeneration::visit(AST::WhileDoExprNode* /*node*/) {}
void CodeGeneration::visit(AST::LetExprNode* /*node*/) {}
void CodeGeneration::visit(AST::AssignExprNode* /*node*/) {}
void CodeGeneration::visit(AST::BinaryExprNode* /*node*/) {}
void CodeGeneration::visit(AST::AndNode* /*node*/) {}
void CodeGeneration::visit(AST::EqualNode* /*node*/) {}
void CodeGeneration::visit(AST::LowerNode* /*node*/) {}
void CodeGeneration::visit(AST::LowerEqualNode* /*node*/) {}
void CodeGeneration::visit(AST::PlusNode* /*node*/) {}
void CodeGeneration::visit(AST::MinusNode* /*node*/) {}
void CodeGeneration::visit(AST::TimesNode* /*node*/) {}
void CodeGeneration::visit(AST::DivNode* /*node*/) {}
void CodeGeneration::visit(AST::PowNode* /*node*/) {}
void CodeGeneration::visit(AST::UnaryExprNode* /*node*/) {}
void CodeGeneration::visit(AST::UnaryMinusNode* /*node*/) {}
void CodeGeneration::visit(AST::NotNode* /*node*/) {}
void CodeGeneration::visit(AST::IsNullNode* /*node*/) {}
void CodeGeneration::visit(AST::NewExprNode* /*node*/) {}
void CodeGeneration::visit(AST::VariableExprNode* /*node*/) {}
void CodeGeneration::visit(AST::UnitExprNode* /*node*/) {}
void CodeGeneration::visit(AST::ParExprNode* /*node*/) {}
void CodeGeneration::visit(AST::BlockNode* /*node*/) {}
void CodeGeneration::visit(AST::BlockExprNode* /*node*/) {}
void CodeGeneration::visit(AST::ArgsNode* /*node*/) {}
void CodeGeneration::visit(AST::MethodNode* /*node*/) {}
void CodeGeneration::visit(AST::CallExprNode* /*node*/) {}
void CodeGeneration::visit(AST::IntegerLiteralNode* /*node*/) {}
void CodeGeneration::visit(AST::LiteralNode* /*node*/) {}
void CodeGeneration::visit(AST::BooleanLiteralNode* /*node*/) {}
