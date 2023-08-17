#ifndef FOURTH_PASS_HPP
#define FOURTH_PASS_HPP

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "ast.hpp"
#include "firstPass.hpp"

using namespace AST;
using namespace std;

class FourthPass : public AST::Visitor {
    public:
        FourthPass(const std::unordered_map<std::string, FirstPass::ClassInfo>& classInfoMap, const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classMethodTypes, const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classFieldTypes, const std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> classMethodFormalsTypes);

        void visit(AST::ProgramNode* node) override;
        void visit(AST::TypeNode* node) override;
        void visit(AST::FormalNode* node) override;
        void visit(AST::FormalsNode* node) override;
        void visit(AST::ExprNode* node) override;
        void visit(AST::IfThenElseExprNode* node) override;
        void visit(AST::WhileDoExprNode* node) override;
        void visit(AST::LetExprNode* node) override;
        void visit(AST::AssignExprNode* node) override;
        void visit(AST::BinaryExprNode* node) override;
        void visit(AST::AndNode* node) override;
        void visit(AST::EqualNode* node) override;
        void visit(AST::LowerNode* node) override;
        void visit(AST::LowerEqualNode* node) override;
        void visit(AST::PlusNode* node) override;
        void visit(AST::MinusNode* node) override;
        void visit(AST::TimesNode* node) override;
        void visit(AST::DivNode* node) override;
        void visit(AST::PowNode* node) override;
        void visit(AST::UnaryExprNode* node) override;

        void visit(AST::UnaryMinusNode* node) override;
        void visit(AST::NotNode* node) override;
        void visit(AST::IsNullNode* node) override;

        void visit(AST::NewExprNode* node) override;
        void visit(AST::VariableExprNode* node) override;
        void visit(AST::UnitExprNode* node) override;
        void visit(AST::ParExprNode* node) override;
        void visit(AST::BlockNode* node) override;
        void visit(AST::BlockExprNode* node) override;
        void visit(AST::ArgsNode* node) override;
        void visit(AST::FieldNode* node) override;
        void visit(AST::MethodNode* node) override;
        void visit(AST::ClassBodyNode* node) override;
        void visit(AST::ClassNode* node) override;
        void visit(AST::CallExprNode* node) override;
        void visit(AST::IntegerLiteralNode* node) override;
        void visit(AST::LiteralNode* node) override;
        void visit(AST::BooleanLiteralNode* node) override;

    private:
        const std::unordered_map<std::string, FirstPass::ClassInfo>& classInfoMap_;

        // Data structure to store class names, method names, field names, and their respective types
        const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classMethodTypes_;
        const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classFieldTypes_;

        // Data structure to store class names, method names, formals names and their respective types
        const std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>
        classMethodFormalsTypes_;

        static const std::unordered_map<std::string, Type> type_map_;

        bool isClassDefined(const std::string& className);
        bool isTypeDefined(const AST::Type& type);
        bool isMethodDefined(const std::string& className, const std::string& methodName);
        
        std::string currentClassName_;
        std::string currentMethodName_;
        // std::string currentFormalName_;
        std::string currentLetVar_;
        AST::Type currentTypeLetVar_;
        std::string currentLetTypename_;
        bool inField_;

};

#endif // FOURTH_PASS_HPP
