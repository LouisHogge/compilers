#ifndef CODE_GENERATION_HPP
#define CODE_GENERATION_HPP

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/raw_os_ostream.h>

#include <fstream>
#include <stack>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <string>
#include "ast.hpp"
#include "firstPass.hpp"
#include <mutex>

class Program; // forward declaration of Program

class CodeGeneration : public AST::Visitor {
public:
    CodeGeneration(const std::unordered_map<std::string, FirstPass::ClassInfo>& classInfoMap, const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classMethodTypes, const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classFieldTypes, const std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> classMethodFormalsTypes);

    static CodeGeneration* getInstance(const std::string &fileName);
    void createLLVMFunction(llvm::Type* returnType, const std::string& functionName, const std::vector<llvm::Type*>& argTypes);
    llvm::Type* getType(const std::string type);
    void declareStructuresAndVtables();
    void declareMallocFunction();
    void declarePowerFunction();
    void declareObjectClassMethods();
    void declareClassMethods();
    void declareInheritedMethods();
    void declareInheritedFields();
    void displayIROnStdout();
    void generateExecutable();
    void cleanup();

    llvm::Type* getInt32TyW(llvm::LLVMContext* context);
    llvm::Type* getInt1TyW(llvm::LLVMContext* context);
    llvm::Type* getInt8PtrTyW(llvm::LLVMContext* context);

    void visit(AST::ProgramNode* node) override;
    void visit(AST::ClassNode* node) override;
    void visit(AST::ClassBodyNode* node) override;
    void visit(AST::FieldNode* node) override;
    void visit(AST::MethodNode* node) override;
    void visit(AST::FormalsNode* node) override;
    void visit(AST::FormalNode* node) override;
    void visit(AST::BlockNode* node) override;
    void visit(AST::ExprNode* node) override;
    void visit(AST::TypeNode* node) override;
    void visit(AST::IfThenElseExprNode* node) override;
    void visit(AST::WhileDoExprNode* node) override;
    void visit(AST::LetExprNode* node) override;
    void visit(AST::AssignExprNode* node) override;
    void visit(AST::BinaryExprNode* node) override;
    void visit(AST::UnaryExprNode* node) override;
    void visit(AST::UnaryMinusNode* node) override;
    void visit(AST::NotNode* node) override;
    void visit(AST::IsNullNode* node) override;
    void visit(AST::ParExprNode* node) override;
    void visit(AST::BlockExprNode* node) override;
    void visit(AST::ArgsNode* node) override;
    void visit(AST::CallExprNode* node) override;

    // add getExpr() method ?
    void visit(AST::AndNode* node) override;
    void visit(AST::EqualNode* node) override;
    void visit(AST::LowerNode* node) override;
    void visit(AST::LowerEqualNode* node) override;
    void visit(AST::PlusNode* node) override;
    void visit(AST::MinusNode* node) override;
    void visit(AST::TimesNode* node) override;
    void visit(AST::DivNode* node) override;
    void visit(AST::PowNode* node) override;
    void visit(AST::NewExprNode* node) override;
    void visit(AST::VariableExprNode* node) override;
    void visit(AST::UnitExprNode* node) override;
    void visit(AST::IntegerLiteralNode* node) override;
    void visit(AST::LiteralNode* node) override;
    void visit(AST::BooleanLiteralNode* node) override;

private:
    std::unique_ptr<llvm::LLVMContext> llvmContext;
    std::unique_ptr<llvm::Module> llvmModule;
    std::unique_ptr<llvm::IRBuilder<>> llvmBuilder;

    AST::ProgramNode* currentProgramNode_;
    std::string currentFileName_;
    std::string currentClassName_;

    const std::unordered_map<std::string, FirstPass::ClassInfo>& classInfoMap_;
    
    // Data structure to store class names, method names, field names, and their respective types
    const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classMethodTypes_;
    const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classFieldTypes_;

    // Data structure to store class names, method names, formals names and their respective types
    const std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> classMethodFormalsTypes_;

    std::unordered_map<std::string, llvm::Type* (*)(llvm::LLVMContext*)> type_map_;

};

#endif // CODE_GENERATION_HPP