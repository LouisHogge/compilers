#include "thirdPass.hpp"
#include "utils.hpp"

/*
    Third pass to report any overridden methods or fields, to check main method, to report any undefined types and to record the types of formals for each class
*/
const std::unordered_map<std::string, AST::Type> ThirdPass::type_map_ = {
    {"int32", AST::Type::INT32},
    {"bool", AST::Type::BOOL},
    {"string", AST::Type::STRING},
    {"unit", AST::Type::UNIT}
};

ThirdPass::ThirdPass(const std::unordered_map<std::string, FirstPass::ClassInfo>& classInfoMap,
const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classMethodTypes,
const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classFieldTypes)
: classInfoMap_(classInfoMap), classMethodTypes_(classMethodTypes), classFieldTypes_(classFieldTypes) {}

// ProgramNode --> ClassNode and check
void ThirdPass::visit(AST::ProgramNode* node) {

    for (auto& classNode : node->getClasses()) {
        classNode->accept(this);
    }
    
    checkMainMethod(node);
}

// ClassNode --> ClassBodyNode
void ThirdPass::visit(AST::ClassNode* node) {
    currentClassName_ = node->getName();

    AST::ClassBodyNode* classBodyNode = node->getClassBody();
    classBodyNode->accept(this);
}

// ClassBodyNode --> MethodNode and FieldNode
void ThirdPass::visit(AST::ClassBodyNode* node) {
    for (auto& methodNode : node->getMethods()) {
        methodNode->accept(this);
    }

    for (auto& fieldNode : node->getFields()) {
        fieldNode->accept(this);
    }
}

// FieldNode --> TypeNode and ExprNode and check
void ThirdPass::visit(AST::FieldNode* node) {
    
    if (ancestorFieldCheck(currentClassName_, node)) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Field " + node->getName() + " is already defined in an ancestor of class " + currentClassName_);
    }

    AST::TypeNode* typeNode = node->getType();
    if (typeNode != NULL) {
        typeNode->accept(this);
    }

    AST::ExprNode* initExprNode = node->getInitExpr();
    if (initExprNode != NULL) {
        initExprNode->accept(this);
    }
}

// MethodNode --> FormalsNode and BlockNode and check
void ThirdPass::visit(AST::MethodNode* node) {
    currentMethodName_ = node->getName();
    
    int check = ancestorMethodCheck(currentClassName_, node);

    if (check == 1) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Overridden method " + currentMethodName_ + " in class " + currentClassName_ + " has a different return type than the method in the ancestor class.");
    } else if (check == 2) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Overridden method " + currentMethodName_ + " in class " + currentClassName_ + " has a different number of formal arguments than the method in the ancestor class.");
    } else if (check == 3) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Overridden method " + currentMethodName_ + " in class " + currentClassName_ + " has a formal argument with a different type than the method in the ancestor class.");
    } else if (check == 4) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Overridden method " + currentMethodName_ + " in class " + currentClassName_ + " has a formal argument with a different name than the method in the ancestor class.");
    }

    AST::TypeNode* retTypeNode =  node->getRetType();
    if (retTypeNode != NULL) {
        retTypeNode->accept(this);
    }

    AST::FormalsNode* formalsNode = node->getFormals();
    if (formalsNode != NULL) {
        formalsNode->accept(this);
    }

    AST::BlockNode* blockNode = node->getBlock();
    if (blockNode != NULL) {
        blockNode->accept(this);
    }
}

// FormalsNode --> FormalNode
void ThirdPass::visit(AST::FormalsNode* node) {
    for (auto& formalsNode : node->getFormals()) {
        formalsNode->accept(this);
    }
}

// FormalNode --> classMethodFormalTypes_ + TypeNode
void ThirdPass::visit(AST::FormalNode* node) {
    
    AST::TypeNode* formalTypeNode = node->getType();
    if (formalTypeNode != NULL) {
        formalTypeNode->accept(this);
    }

    std::string formalName = node->getName();
    std::string formalType = formalTypeNode->getTypeStr();

    // check if currentClassName_ in classMethodFormalTypes_
    auto classIt = classMethodFormalTypes_.find(currentClassName_);
    if (classIt != classMethodFormalTypes_.end()) {

        // check if currentMethodName_ in classMethodFormalTypes_ at currentClassName_
        auto methodIt = classIt->second.find(currentMethodName_);
        if (methodIt != classIt->second.end()) {

            // check if formalName in classMethodFormalTypes_ at currentClassName_ at currentMethodName_
            if (methodIt->second.find(formalName) != methodIt->second.end()) {
                error(formalTypeNode->getFilename(), formalTypeNode->getLine(), formalTypeNode->getColumn(), "Formal " + formalName + " is already defined");
            }
        }
    }

    classMethodFormalTypes_[currentClassName_][currentMethodName_][formalName] = formalType;
}

// BlockNode --> ExprNode
void ThirdPass::visit(AST::BlockNode* node) {
    for (auto& exprNode : node->getExpr()) {
        exprNode->accept(this);
    }
}

// IfThenElseExprNode --> ExprNode
void ThirdPass::visit(AST::IfThenElseExprNode* node) {
    AST::ExprNode* condExprNode = node->getCondExpr();
    if (condExprNode != NULL) {
        condExprNode->accept(this);
    }
    
    AST::ExprNode* thenExprNode = node->getThenExpr();
    if (thenExprNode != NULL) {
        thenExprNode->accept(this);
    }

    AST::ExprNode* elseExprNode = node->getElseExpr();
    if (elseExprNode != NULL) {
        elseExprNode->accept(this);
    }
}

// WhileDoExprNode --> ExprNode
void ThirdPass::visit(AST::WhileDoExprNode* node) {
    AST::ExprNode* condExprNode = node->getCondExpr();
    if (condExprNode != NULL) {
        condExprNode->accept(this);
    }

    AST::ExprNode* bodyExprNode = node->getBodyExpr();
    if (bodyExprNode != NULL) {
        bodyExprNode->accept(this);
    }
}

// LetExprNode --> TypeNode and ExprNode
void ThirdPass::visit(AST::LetExprNode* node) {
    AST::ExprNode* initExprNode = node->getInitExpr();
    if (initExprNode != NULL) {
        initExprNode->accept(this);
    }
    AST::TypeNode* typeNode = node->getTypeNode();
    if (typeNode != NULL) {
        typeNode->accept(this);
    }

    AST::ExprNode* scopeExprNode = node->getScopeExpr();
    if (scopeExprNode != NULL) {
        scopeExprNode->accept(this);
    }
}

// AssignExprNode --> ExprNode
void ThirdPass::visit(AST::AssignExprNode* node) {
    AST::ExprNode* exprNode = node->getExpr();
    if (exprNode != NULL) {
        exprNode->accept(this);
    }
}

// BinaryExprNode --> ExprNode
void ThirdPass::visit(AST::BinaryExprNode* node) {
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }
}

// UnaryExprNode --> ExprNode
void ThirdPass::visit(AST::UnaryExprNode* node) {
    AST::ExprNode* exprNode = node->getNode();
    if (exprNode != NULL) {
        exprNode->accept(this);
    }
}

// ParExprNode --> ExprNode
void ThirdPass::visit(AST::ParExprNode* node) {
    AST::ExprNode* exprNode = node->getExpr();
    if (exprNode != NULL) {
        exprNode->accept(this);
    }
}

// BlockExprNode --> BlockNode
void ThirdPass::visit(AST::BlockExprNode* node) {
    AST::BlockNode* blockNode = node->getBlock();
    if (blockNode != NULL) {
        blockNode->accept(this);
    }
}

// ArgsNode --> ExprNode
void ThirdPass::visit(AST::ArgsNode* node) {
    for (auto& exprsNode : node->getExprs()) {
        exprsNode->accept(this);
    }
}

// CallExprNode --> ExprNode and ArgsNode*
void ThirdPass::visit(AST::CallExprNode* node) {
    AST::ExprNode* objExprNode = node->getObjExpr();
    if (objExprNode != NULL) {
        objExprNode->accept(this);
    }

    AST::ArgsNode* exprListNode = node->getExprList();
    if (exprListNode != NULL) {
        exprListNode->accept(this);
    }
}

// AndNode --> ExprNode
void ThirdPass::visit(AST::AndNode* node) {
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }
}

// EqualNode --> ExprNode
void ThirdPass::visit(AST::EqualNode* node) {
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }
}

// LowerNode --> ExprNode
void ThirdPass::visit(AST::LowerNode* node) {
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }
}

// LowerEqualNode --> ExprNode
void ThirdPass::visit(AST::LowerEqualNode* node) {
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }
}

// PlusNode --> ExprNode
void ThirdPass::visit(AST::PlusNode* node) {
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }
}

// MinusNode --> ExprNode
void ThirdPass::visit(AST::MinusNode* node) {
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }
}

// TimesNode --> ExprNode
void ThirdPass::visit(AST::TimesNode* node) {
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }
}

// DivNode --> ExprNode
void ThirdPass::visit(AST::DivNode* node) {
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }
}

// PowNode --> ExprNode
void ThirdPass::visit(AST::PowNode* node) {
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }
}

int ThirdPass::ancestorMethodCheck(const std::string& className, AST::MethodNode* methodNode) {
    std::string currentClass = className;
    std::string methodName = methodNode->getName();

    // loop walks up class hierarchy
    while (classInfoMap_.find(currentClass) != classInfoMap_.end()) {

        // check if currentClass in classMethodTypes_
        if (classMethodTypes_.find(currentClass) != classMethodTypes_.end()) {

            // check if methodName in classMethodTypes_ at currentClass --> does a method with same name exists in current class?
            if (classMethodTypes_.at(currentClass).find(methodName) != classMethodTypes_.at(currentClass).end()) {

                // Retrieve the matching ancestor method
                AST::ClassNode* ancestorClassNode = classInfoMap_.at(currentClass).classNode;
                AST::ClassBodyNode* ancestorClassBodyNode = ancestorClassNode->getClassBody();
                AST::MethodNode* ancestorMethodNode = nullptr;

                for (auto& method : ancestorClassBodyNode->getMethods()) {
                    if (method->getName() == methodName) {
                        ancestorMethodNode = method;
                        break;
                    }
                }
                ancestorMethodNode->getRetType()->accept(this);
                methodNode->getRetType()->accept(this);

                if (ancestorMethodNode->getRetType()->getType() != methodNode->getRetType()->getType()) {
                    return 1;
                }

                if (ancestorMethodNode->getFormals()->getFormals().size() != methodNode->getFormals()->getFormals().size()) {
                    return 2;
                }

                for (size_t i = 0; i < ancestorMethodNode->getFormals()->getFormals().size(); ++i) {
                    ancestorMethodNode->getFormals()->getFormals()[i]->getType()->accept(this);
                    methodNode->getFormals()->getFormals()[i]->getType()->accept(this);
                    if (ancestorMethodNode->getFormals()->getFormals()[i]->getType()->getType()
                    != methodNode->getFormals()->getFormals()[i]->getType()->getType()) {
                        return 3;
                    }

                    if (ancestorMethodNode->getFormals()->getFormals()[i]->getName()
                    != methodNode->getFormals()->getFormals()[i]->getName()) {
                        return 4;
                    }
                }
            }
        }
        currentClass = classInfoMap_.at(currentClass).parentClassName;
    }
    
    return 0;
}

bool ThirdPass::ancestorFieldCheck(const std::string& className, AST::FieldNode* fieldNode) {
    std::string currentClass = className;
    std::string fieldName = fieldNode->getName();

    // loop walks up class hierarchy
    while (classInfoMap_.find(currentClass) != classInfoMap_.end()) {

        // check if currentClass in classFieldTypes_
        if (classFieldTypes_.find(currentClass) != classFieldTypes_.end()) {

            // check if fieldName in classFieldTypes_ at currentClass --> does a field with same name exists in current class?
            if (classFieldTypes_.at(currentClass).find(fieldName) != classFieldTypes_.at(currentClass).end()) {

                // Retrieve the matching ancestor field
                AST::ClassNode* ancestorClassNode = classInfoMap_.at(currentClass).classNode;
                AST::ClassBodyNode* ancestorClassBodyNode = ancestorClassNode->getClassBody();
                AST::FieldNode* ancestorFieldNode = nullptr;

                for (auto& field : ancestorClassBodyNode->getFields()) {
                    if (field->getName() == fieldName) {
                        ancestorFieldNode = field;
                        break;
                    }
                }

                if (ancestorFieldNode != fieldNode) {
                    return true;
                }
            }
        }
        currentClass = classInfoMap_.at(currentClass).parentClassName;
    }
    return false;
}

void ThirdPass::checkMainMethod(AST::ProgramNode* node) {
    bool mainMethodFound = false;

    auto mainClassMethodsIt = classMethodTypes_.find("Main");
    if (mainClassMethodsIt != classMethodTypes_.end()) {
        auto mainMethodIt = mainClassMethodsIt->second.find("main");

        if (mainMethodIt != mainClassMethodsIt->second.end()) {
            mainMethodFound = true;

            if (mainMethodIt->second != "int32") {
                error(node->getFilename(), node->getLine(), node->getColumn(),
                "Main method should have a return type of int32");
            }

            // Find the Main class node
            AST::ClassNode* mainClassNode = nullptr;
            
            for (auto& classNode : node->getClasses()) {
                if (classNode->getName() == "Main") {
                    mainClassNode = classNode;
                    break;
                }
            }

            // Check if the Main method has no arguments
            if (mainClassNode != nullptr) {
                AST::ClassBodyNode* classBodyNode = mainClassNode->getClassBody();
                for (auto& methodNode : classBodyNode->getMethods()) {
                    if (methodNode->getName() == "main") {
                        if (!methodNode->getFormals()->getFormals().empty()) {
                            error(node->getFilename(), node->getLine(), node->getColumn(),
                            "Main method should have no arguments");
                        }
                        break;
                    }
                }
            }
        }
    }

    if (!mainMethodFound) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Main method not found in the Main class");
    }
}

// TypeNode --> check
void ThirdPass::visit(AST::TypeNode* node) {
    
    const auto it = type_map_.find(node->getTypeStr());
    if (it != type_map_.end()) {
        node->setType(it->second);
        node->setTypeStr(it->first);

    } else {
        node->setType(AST::Type::CLASS);
        if (node->getTypeStr() == "")
            node->setTypeStr(currentClassName_);
    }

    AST::Type type = node->getType();
    std::string typeName = node->getTypeStr();

    // Check
    // if (!isTypeDefined(type) && isClassDefined(typeName)) {
    if (!isTypeDefined(type)) {
        error(node->getFilename(), node->getLine(), node->getColumn(), "Undefined type " + typeName);
    }

    // Check if class is undefined
    if (type == AST::Type::CLASS && !isClassDefined(typeName)) {       
        error(node->getFilename(), node->getLine(), node->getColumn(), "Undefined type " + typeName);
    }
}

bool ThirdPass::isClassDefined(const std::string& className) {
    return classInfoMap_.find(className) != classInfoMap_.end();
}

bool ThirdPass::isTypeDefined(const AST::Type& type) {
    switch(type) {
        case AST::Type::UNIT:
        case AST::Type::BOOL:
        case AST::Type::INT32:
        case AST::Type::STRING:
        case AST::Type::CLASS:
            return true;
        default:
            return false;
    }
}

const std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>& ThirdPass::getClassMethodFormalTypes() const {
    return classMethodFormalTypes_;
}

// unused
void ThirdPass::visit(AST::NewExprNode* /*node*/) {}
void ThirdPass::visit(AST::VariableExprNode* /*node*/) {}
void ThirdPass::visit(AST::UnitExprNode* /*node*/) {}
void ThirdPass::visit(AST::IntegerLiteralNode* /*node*/) {}
void ThirdPass::visit(AST::LiteralNode* /*node*/) {}
void ThirdPass::visit(AST::BooleanLiteralNode* /*node*/) {}
void ThirdPass::visit(AST::UnaryMinusNode* /*node*/) {}
void ThirdPass::visit(AST::NotNode* /*node*/) {}
void ThirdPass::visit(AST::IsNullNode* /*node*/) {}
void ThirdPass::visit(AST::ExprNode* /*node*/) {}
