#include "firstPass.hpp"
#include "utils.hpp"

/*
    First pass over AST, just considering class declarations (ignoring their fields and methods),
    recording which classes are defined, and what their parents are. Then check for cycles.
*/

// ProgramNode --> ClassNode + check cycle
void FirstPass::visit(AST::ProgramNode* node) {
    for (auto& classNode : node->getClasses()) {
        classNode->accept(this);
    }
    
    checkInheritanceCycles(node);
}

// ClassNode --> check
void FirstPass::visit(AST::ClassNode* node) {
    std::string className = node->getName();
    std::string parentClassName = node->getParent();

    // debug
    // std::cout << "1st pass ClassNode className: " + className << std::endl;
    // std::cout << "1st pass ClassNode parentClassName before: " + parentClassName << std::endl;

    if (classInfoMap_.find(className) != classInfoMap_.end()) {
        error(node->getFilename(), node->getLine(), node->getColumn(), "Class " + className + " is already defined.");
    }

    if (className == "Object" && !parentClassName.empty()) {
        error(node->getFilename(), node->getLine(), node->getColumn(), "The predefined Object class cannot have a parent class.");
    }

    // Object class is default parent class
    if (parentClassName == "" && className != "Object") {
        parentClassName = "Object";
        node->setParent("Object");
    }

    // debug
    // std::cout << "1st pass ClassNode parentClassName after: " + parentClassName << std::endl;

    classInfoMap_[className] = {node, parentClassName};
}

bool FirstPass::dfs(const std::string& className, std::unordered_map<std::string, bool>& visited) {
    if (visited.find(className) != visited.end()) {
        return true;
    }

    visited[className] = true;

    ClassInfo& classInfo = classInfoMap_[className];
    if (!classInfo.parentClassName.empty()) {
        if (dfs(classInfo.parentClassName, visited)) {
            return true;
        }
    }

    visited.erase(className);
    return false;
}

void FirstPass::checkInheritanceCycles(AST::ProgramNode* node) {
    std::unordered_map<std::string, bool> visited;

    for (const auto& entry : classInfoMap_) {
        const std::string& className = entry.first;
        
        if (dfs(className, visited)) {
            error(node->getFilename(), node->getLine(), node->getColumn(),
            "Cycle detected in class inheritance involving class " + className);
        }
    }
}

const std::unordered_map<std::string, FirstPass::ClassInfo>& FirstPass::getClassInfoMap() const {
    return classInfoMap_;
}

// unused
void FirstPass::visit(AST::TypeNode* /*node*/) {}
void FirstPass::visit(AST::FormalNode* /*node*/) {}
void FirstPass::visit(AST::FormalsNode* /*node*/) {}
void FirstPass::visit(AST::ExprNode* /*node*/) {}
void FirstPass::visit(AST::IfThenElseExprNode* /*node*/) {}
void FirstPass::visit(AST::WhileDoExprNode* /*node*/) {}
void FirstPass::visit(AST::LetExprNode* /*node*/) {}
void FirstPass::visit(AST::AssignExprNode* /*node*/) {}
void FirstPass::visit(AST::BinaryExprNode* /*node*/) {}
void FirstPass::visit(AST::AndNode* /*node*/) {}
void FirstPass::visit(AST::EqualNode* /*node*/) {}
void FirstPass::visit(AST::LowerNode* /*node*/) {}
void FirstPass::visit(AST::LowerEqualNode* /*node*/) {}
void FirstPass::visit(AST::PlusNode* /*node*/) {}
void FirstPass::visit(AST::MinusNode* /*node*/) {}
void FirstPass::visit(AST::TimesNode* /*node*/) {}
void FirstPass::visit(AST::DivNode* /*node*/) {}
void FirstPass::visit(AST::PowNode* /*node*/) {}
void FirstPass::visit(AST::UnaryExprNode* /*node*/) {}
void FirstPass::visit(AST::UnaryMinusNode* /*node*/) {}
void FirstPass::visit(AST::NotNode* /*node*/) {}
void FirstPass::visit(AST::IsNullNode* /*node*/) {}
void FirstPass::visit(AST::NewExprNode* /*node*/) {}
void FirstPass::visit(AST::VariableExprNode* /*node*/) {}
void FirstPass::visit(AST::UnitExprNode* /*node*/) {}
void FirstPass::visit(AST::ParExprNode* /*node*/) {}
void FirstPass::visit(AST::BlockNode* /*node*/) {}
void FirstPass::visit(AST::BlockExprNode* /*node*/) {}
void FirstPass::visit(AST::ArgsNode* /*node*/) {}
void FirstPass::visit(AST::FieldNode* /*node*/) {}
void FirstPass::visit(AST::MethodNode* /*node*/) {}
void FirstPass::visit(AST::ClassBodyNode* /*node*/) {}
void FirstPass::visit(AST::CallExprNode* /*node*/) {}
void FirstPass::visit(AST::IntegerLiteralNode* /*node*/) {}
void FirstPass::visit(AST::LiteralNode* /*node*/) {}
void FirstPass::visit(AST::BooleanLiteralNode* /*node*/) {}

// debug
void FirstPass::printClassInfoMap(const std::unordered_map<std::string, FirstPass::ClassInfo>& classInfoMap_) {
    for (const auto& classEntry : classInfoMap_) {
        const std::string& className = classEntry.first;
        const FirstPass::ClassInfo& classInfo = classEntry.second;
        
        std::cout << "Class Name: " << className << "\n";
        
        // Print parent class name if it exists
        if (!classInfo.parentClassName.empty()) {
            std::cout << "  Parent Class: " << classInfo.parentClassName << "\n";
        } else {
            std::cout << "  No Parent Class" << "\n";
        }
        
        // If you want to print something about the ClassNode, do so here.
        // For this example, I'll just print its address.
        // std::cout << "  ClassNode Address: " << classInfo.classNode << "\n";
    }
}
