#include "secondPass.hpp"
#include "utils.hpp"

/*
    Second pass, over the whole AST this time, to report any use of an undefined class and to record the types of methods and fields for each class, without inspecting their body or initializer
*/

SecondPass::SecondPass(const std::unordered_map<std::string, FirstPass::ClassInfo>& classInfoMap): classInfoMap_(classInfoMap) {}

// ProgramNode --> ClassNode
void SecondPass::visit(AST::ProgramNode* node) {
    
    for (auto& classNode : node->getClasses()) {
        classNode->accept(this);
    }

    if (!isClassDefined("Main")) {
        error(node->getFilename(), node->getLine(), node->getColumn(), "Undefined Main class");
    }
}
// ClassNode --> ClassBodyNode and check
void SecondPass::visit(AST::ClassNode* node) {
    currentClassName_ = node->getName();
    std::string parentClassName = node->getParent();

    // Check
    if (currentClassName_ != "Object") {
        // Check if parentClassName is defined
        if (!isClassDefined(parentClassName))
            error(node->getFilename(), node->getLine(), node->getColumn(), "Undefined parent class " + parentClassName + " for class " + currentClassName_ = node->getName());
        // Check if parentClassName of parentClassName is defined
        if (parentClassName != "Object" && !isClassDefined(classInfoMap_.at(parentClassName).parentClassName))
            error(node->getFilename(), node->getLine(), node->getColumn(), "Undefined parent class " + parentClassName + " for class " + currentClassName_ = node->getName());
    }

    AST::ClassBodyNode* classBodyNode = node->getClassBody();
    classBodyNode->accept(this);
}

// ClassBodyNode --> MethodNode and FieldNode
void SecondPass::visit(AST::ClassBodyNode* node) {
    for (auto& methodNode : node->getMethods()) {
        methodNode->accept(this);
    }

    for (auto& fieldNode : node->getFields()) {
        fieldNode->accept(this);
    }
}

// MethodNode --> check
void SecondPass::visit(AST::MethodNode* node) {
    std::string methodName = node->getName();
    AST::TypeNode* methodTypeNode = node->getRetType();
    std::string methodType = methodTypeNode->getTypeStr();

    if (classMethodTypes_[currentClassName_].find(methodName) != classMethodTypes_[currentClassName_].end()) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Method " + methodName + " is already defined in class " + currentClassName_);
    }

    classMethodTypes_[currentClassName_][methodName] = methodType;
}

// FieldNode --> check
void SecondPass::visit(AST::FieldNode* node) {
    std::string fieldName = node->getName();
    AST::TypeNode* fieldTypeNode = node->getType();
    std::string fieldType = fieldTypeNode->getTypeStr();

    if (classFieldTypes_[currentClassName_].find(fieldName) != classFieldTypes_[currentClassName_].end()) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Field " + fieldName + " is already defined in class " + currentClassName_);
    }

    classFieldTypes_[currentClassName_][fieldName] = fieldType;
}

bool SecondPass::isClassDefined(const std::string& className) {
    return classInfoMap_.find(className) != classInfoMap_.end();
}

const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& SecondPass::getClassMethodTypes() const {
    return classMethodTypes_;
}

const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& SecondPass::getClassFieldTypes() const {
    return classFieldTypes_;
}

// unused
void SecondPass::visit(AST::TypeNode* /*node*/) {}
void SecondPass::visit(AST::FormalNode* /*node*/) {}
void SecondPass::visit(AST::FormalsNode* /*node*/) {}
void SecondPass::visit(AST::ExprNode* /*node*/) {}
void SecondPass::visit(AST::IfThenElseExprNode* /*node*/) {}
void SecondPass::visit(AST::WhileDoExprNode* /*node*/) {}
void SecondPass::visit(AST::LetExprNode* /*node*/) {}
void SecondPass::visit(AST::AssignExprNode* /*node*/) {}
void SecondPass::visit(AST::BinaryExprNode* /*node*/) {}
void SecondPass::visit(AST::AndNode* /*node*/) {}
void SecondPass::visit(AST::EqualNode* /*node*/) {}
void SecondPass::visit(AST::LowerNode* /*node*/) {}
void SecondPass::visit(AST::LowerEqualNode* /*node*/) {}
void SecondPass::visit(AST::PlusNode* /*node*/) {}
void SecondPass::visit(AST::MinusNode* /*node*/) {}
void SecondPass::visit(AST::TimesNode* /*node*/) {}
void SecondPass::visit(AST::DivNode* /*node*/) {}
void SecondPass::visit(AST::PowNode* /*node*/) {}
void SecondPass::visit(AST::UnaryExprNode* /*node*/) {}
void SecondPass::visit(AST::UnaryMinusNode* /*node*/) {}
void SecondPass::visit(AST::NotNode* /*node*/) {}
void SecondPass::visit(AST::IsNullNode* /*node*/) {}
void SecondPass::visit(AST::NewExprNode* /*node*/) {}
void SecondPass::visit(AST::VariableExprNode* /*node*/) {}
void SecondPass::visit(AST::UnitExprNode* /*node*/) {}
void SecondPass::visit(AST::ParExprNode* /*node*/) {}
void SecondPass::visit(AST::BlockNode* /*node*/) {}
void SecondPass::visit(AST::BlockExprNode* /*node*/) {}
void SecondPass::visit(AST::ArgsNode* /*node*/) {}
void SecondPass::visit(AST::CallExprNode* /*node*/) {}
void SecondPass::visit(AST::IntegerLiteralNode* /*node*/) {}
void SecondPass::visit(AST::LiteralNode* /*node*/) {}
void SecondPass::visit(AST::BooleanLiteralNode* /*node*/) {}
