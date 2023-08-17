#include <algorithm>

#include "fourthPass.hpp"
#include "utils.hpp"

/*
    Fourth pass to perform type checking
*/

const std::unordered_map<std::string, AST::Type> FourthPass::type_map_ = {
    {"int32", AST::Type::INT32},
    {"bool", AST::Type::BOOL},
    {"string", AST::Type::STRING},
    {"unit", AST::Type::UNIT}
};

FourthPass::FourthPass(const std::unordered_map<std::string, FirstPass::ClassInfo>& classInfoMap, const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classMethodTypes, const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> classFieldTypes, const std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> classMethodFormalsTypes): classInfoMap_(classInfoMap), classMethodTypes_(classMethodTypes), classFieldTypes_(classFieldTypes), classMethodFormalsTypes_(classMethodFormalsTypes) {}

// ProgramNode --> ClassNode
void FourthPass::visit(ProgramNode* node) {
    
    // Visit each class node in the program
    for (auto& class_node : node->getClasses()) {
        class_node->accept(this);
    }
}

// ClassNode --> ClassBodyNode
void FourthPass::visit(ClassNode* node) {
    currentClassName_ = node->getName();

    AST::ClassBodyNode* classBodyNode = node->getClassBody();
    if (classBodyNode != NULL) {
        classBodyNode->accept(this);
    }
    
    node->setType(Type::CLASS);
}

void FourthPass::visit(TypeNode* /*node*/) {}

// FormalsNode --> FormalNode
void FourthPass::visit(FormalsNode* node) {
    // Visit each formal node in formals
    for (auto& formal_node : node->getFormals()) {
        formal_node->accept(this);
    }
}

// FormalNode --> TypeNode
void FourthPass::visit(FormalNode* node) {
    
    AST::TypeNode* typeNode = node->getType();
    if (typeNode != NULL) {
        typeNode->accept(this);
    }
}

void FourthPass::visit(ExprNode* /*node*/) {}

// IfThenElseExprNode --> ExprNode
void FourthPass::visit(IfThenElseExprNode* node) {
    
    // evaluate the cond of the loop
    AST::ExprNode* condExprNode = node->getCondExpr();
    if (condExprNode != NULL) {
        condExprNode->accept(this);
    
        // Check if cond is bool
        AST::Type condType = condExprNode->getType();
        if (condType != Type::BOOL) {
            error(node->getFilename(), node->getLine(), node->getColumn(),
            "Condition must be of type bool in If Then");
        }
    }

    // evaluate the branches
    AST::ExprNode* thenExprNode = node->getThenExpr();
    thenExprNode->accept(this);
    

    AST::ExprNode* elseExprNode = node->getElseExpr();
    if (elseExprNode != NULL) {
        elseExprNode->accept(this); // if present

        AST::Type thenType = thenExprNode->getType();
        AST::Type elseType = elseExprNode->getType();

        // Both branch of same type
        if (thenType == elseType) {
            node->setType(thenType);

            if (thenType == Type::BOOL)
                node->setTypename("bool");
            if (thenType == Type::STRING)
                node->setTypename("string");
            if (thenType == Type::INT32)
                node->setTypename("int32");

            // type of the first common ancestor of the two branches
            if (thenType == Type::CLASS) {
                if (elseExprNode->getTypename() == thenExprNode->getTypename()) {
                    node->setTypename(elseExprNode->getTypename());
                } else {
                    std::string classElse = elseExprNode->getTypename();
                    std::string classThen = thenExprNode->getTypename();

                    std::unordered_set<std::string> ancestors1;
                    std::unordered_set<std::string> ancestors2;

                    // Stock all ancestor of first class
                    std::string currentClass = classElse;
                    while (currentClass != "Object") {
                        ancestors1.insert(currentClass);
                        currentClass = classInfoMap_.at(currentClass).parentClassName;
                    }

                    // Stock all ancestor of second class
                    currentClass = classThen;
                    while (currentClass != "Object") {
                        ancestors2.insert(currentClass);
                        currentClass = classInfoMap_.at(currentClass).parentClassName;
                    }

                    // Find first common ancestor
                    currentClass = classElse;
                    while (currentClass != "Object") {
                        if (ancestors2.find(currentClass) != ancestors2.end()) {
                            node->setTypename(currentClass);
                            break;
                        }
                        currentClass = classInfoMap_.at(currentClass).parentClassName;
                    }

                    // First common ancestor is the root class Object
                    if (currentClass == "Object") {
                        node->setTypename("Object");
                    }
                }
            }
        } 

        // At least one branch is unit
        if (thenType == Type::UNIT || elseType == Type::UNIT) {
            node->setType(Type::UNIT);
            node->setTypename("unit");
            return;
        }
        // error
        if (thenType != elseType) {
            error(node->getFilename(), node->getLine(), node->getColumn(),
            "Then expression and Else expression must be of the same type in If Then");
        }
    } else {
        /*If (at least) one branch has type unit, the types agree and the resulting type of the condi-
            tional is unit
            A conditional of the form
            if <cond> then <expr_t>
            without an else branch is just a shortcut for
            if <cond> then <expr_t> else ()
        */
        node->setType(Type::UNIT);
        node->setTypename("unit");  
    }
}

// WhileDoExprNode --> ExprNode
void FourthPass::visit(WhileDoExprNode* node) {
    
    // evaluate the cond of the loop
    AST::ExprNode* condExprNode = node->getCondExpr();
    if (condExprNode != NULL) {
        condExprNode->accept(this);

        // Check if cond is bool
        AST::Type condType = condExprNode->getType();
        if (condType != Type::BOOL) {
            error(node->getFilename(), node->getLine(), node->getColumn(), "Condition must be of type bool in While");
        }
    }

    
    // evaluate the body of the loop
    AST::ExprNode* bodyExprNode = node->getBodyExpr();
    if (bodyExprNode != NULL) {
        bodyExprNode->accept(this);
    }
    
    // set type of the loop to unit
    node->setType(Type::UNIT);
    node->setTypename("unit");
}

// LetExprNode --> TypeNode and ExprNode
void FourthPass::visit(LetExprNode* node) {
    
    currentLetVar_= node->getName();
    node->getTypeNode()->accept(this);
    currentTypeLetVar_ = node->getTypeNode()->getType();
    currentLetTypename_ = node->getTypeNode()->getTypeStr();
    
    AST::ExprNode* initExprNode = node->getInitExpr();
    if (initExprNode != NULL) {
        
        initExprNode->accept(this);

        AST::TypeNode* typeNode = node->getTypeNode();
        typeNode->accept(this);

        if (typeNode->getType() != Type::CLASS){
            if (initExprNode->getType() != typeNode->getType()) {
                error(node->getFilename(), node->getLine(), node->getColumn(),
                "Init expression and Type expression must be of the same type in Let");
            }
        } else {
            if (initExprNode->getType() == Type::CLASS) {
                std::string name = initExprNode->getTypename();
                auto parentIt = classInfoMap_.find(initExprNode->getTypename());
                if (parentIt != classInfoMap_.end()) {
                    name = parentIt->second.parentClassName;            
                }
            
                if (name != typeNode->getTypeStr()) {
                    error(node->getFilename(), node->getLine(), node->getColumn(),
                    "Init expression and Type expression must be of the same type in Let");
                }
            }
        }

        AST::ExprNode* scopeExprNode = node->getScopeExpr();
        if (scopeExprNode != NULL) {
            scopeExprNode->accept(this);

            if (scopeExprNode->getType() == Type::CLASS) {
                node->setType(scopeExprNode->getType());
                node->setTypename(scopeExprNode->getTypename());
            } else {
                node->setType(scopeExprNode->getType());
                node->setTypename(scopeExprNode->getTypename());
            }
        }
    } else {
        
        AST::ExprNode* scopeExprNode = node->getScopeExpr();
        if (scopeExprNode != NULL) {
            scopeExprNode->accept(this);

            if (scopeExprNode->getType() == Type::CLASS) {
                node->setType(scopeExprNode->getType());
                node->setTypename(scopeExprNode->getTypename());
            } else {
                node->setType(scopeExprNode->getType());
                node->setTypename(scopeExprNode->getTypename());
            }
        }
    }
}

// AssignExprNode --> ExprNode
void FourthPass::visit(AssignExprNode* node) {
    
    AST::ExprNode* variable = node->getName();
    variable->accept(this);

    AST::ExprNode* exprNode = node->getExpr();
    if (exprNode != NULL) {
        exprNode->accept(this);

        auto classIt =  classMethodFormalsTypes_.find(currentClassName_);
        if (classIt != classMethodFormalsTypes_.end()) {
            if ((currentClassName_ == "Main" && currentMethodName_ == "main")
                || (currentClassName_ != "Main" && currentMethodName_ != "main")) {

                auto methodIt = classIt->second.find(currentMethodName_);
                if (methodIt != classIt->second.end()) {

                    auto formalIt = methodIt->second.find(node->getNameStr());
                    if (formalIt != methodIt->second.end()) {

                        Type idType = Type::CLASS;

                        const auto it = type_map_.find(formalIt->second);
                        if (it != type_map_.end()) {
                            idType = it->second;
                        } else {
                            idType = Type::CLASS;
                        }

                        if (exprNode->getType() != idType) {
                            error(node->getFilename(), node->getLine(), node->getColumn(),
                            "Expression must be of same type as name type in Assign");
                        }

                        if (exprNode->getType() == Type::CLASS) {
                            node->setType(exprNode->getType());
                            node->setTypename(exprNode->getTypename());
                        } else {
                            node->setType(exprNode->getType());
                            node->setTypename(exprNode->getTypename());
                        }
                    }
                }
            }
        }
        if (exprNode->getType() == Type::CLASS) {
            node->setType(exprNode->getType());
            node->setTypename(exprNode->getTypename());
        } else {
            node->setType(exprNode->getType());
            node->setTypename(exprNode->getTypename());
        }
    }
}

// BinaryExprNode --> ExprNode
void FourthPass::visit(BinaryExprNode* /*node*/) {}

// AndNode --> ExprNode
void FourthPass::visit(AndNode* node) {
    
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this); // if true, evalutate right

        if (node->getLeftExpr()->getType() != Type::BOOL) {
            error(node->getFilename(), node->getLine(), node->getColumn(),
            "Expression in And must be of type bool");
        }
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    
    
        if (node->getRightExpr()->getType() != Type::BOOL) {
            error(node->getFilename(), node->getLine(), node->getColumn(),
            "Expression in And must be of type bool");
        }
    }

    node->setType(Type::BOOL);
    node->setTypename("bool");
}

// EqualNode --> ExprNode
void FourthPass::visit(EqualNode* node) {
    
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }

    if (leftExprNode->getType() != rightExprNode->getType()) {
        if (leftExprNode->getType() == Type::CLASS || rightExprNode->getType() == Type::CLASS) {
            error(node->getFilename(), node->getLine(), node->getColumn(),
            "error class + primitive type");
        }
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "error different primitive types");
    }

    node->setType(Type::BOOL);
    node->setTypename("bool");
}

// LowerNode --> ExprNode
void FourthPass::visit(LowerNode* node) {
    
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }

    if (leftExprNode->getType() != Type::INT32 || rightExprNode->getType() != Type::INT32) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Expression must be of type int32 in < ");
    }

    node->setType(Type::BOOL);
    node->setTypename("bool");
}

// LowerEqualNode --> ExprNode
void FourthPass::visit(LowerEqualNode* node) {
    
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }

    if (leftExprNode->getType() != Type::INT32 || rightExprNode->getType() != Type::INT32) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Expression must be of type int32 in <= ");
    }

    node->setType(Type::BOOL);
    node->setTypename("bool");
}

// PlusNode --> ExprNode
void FourthPass::visit(PlusNode* node) {
    
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }

    if (leftExprNode->getType() != Type::INT32 || rightExprNode->getType() != Type::INT32) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Expression must be of type int32 in + ");
    }

    node->setType(Type::INT32);
    node->setTypename("int32");
}

// MinusNode --> ExprNode
void FourthPass::visit(MinusNode* node) {
    
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }

    if (leftExprNode->getType() != Type::INT32 || rightExprNode->getType() != Type::INT32) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Expression must be of type int32 in - ");
    }

    node->setType(Type::INT32);
    node->setTypename("int32");
}

// TimesNode --> ExprNode
void FourthPass::visit(TimesNode* node) {
    
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }

    if (leftExprNode->getType() != Type::INT32 || rightExprNode->getType() != Type::INT32) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Expression must be of type int32 in * ");
    }

    node->setType(Type::INT32);
    node->setTypename("int32");
}

// DivNode --> ExprNode
void FourthPass::visit(DivNode* node) {
    
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }

    if (leftExprNode->getType() != Type::INT32 || rightExprNode->getType() != Type::INT32) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Expression must be of type int32 in / ");
    }

    node->setType(Type::INT32);
    node->setTypename("int32");
}

// PowNode --> ExprNode
void FourthPass::visit(PowNode* node) {
    
    AST::ExprNode* leftExprNode = node->getLeftExpr();
    if (leftExprNode != NULL) {
        leftExprNode->accept(this);
    }

    AST::ExprNode* rightExprNode = node->getRightExpr();
    if (rightExprNode != NULL) {
        rightExprNode->accept(this);
    }

    if (leftExprNode->getType() != Type::INT32 || rightExprNode->getType() != Type::INT32) {
        error(node->getFilename(), node->getLine(), node->getColumn(),
        "Expression must be of type int32 in ^ ");
    }

    node->setType(Type::INT32);
    node->setTypename("int32");
}

// UnaryExprNode --> ExprNode
void FourthPass::visit(UnaryExprNode* /*node*/) {}

// UnaryMinusNode --> ExprNode
void FourthPass::visit(UnaryMinusNode* node) {
   
   AST::ExprNode* exprNode = node->getNode();
    if (exprNode != NULL) {
        exprNode->accept(this);
        if (exprNode->getType() != Type::INT32) {
            error(node->getFilename(), node->getLine(), node->getColumn(),
            "expected type is int32");
        }
    }

    node->setType(Type::INT32);
    node->setTypename("int32");
}

// NotNode --> ExprNode
void FourthPass::visit(NotNode* node) {
    
    AST::ExprNode* exprNode = node->getNode();
    if (exprNode != NULL) {
        exprNode->accept(this);
    }

    node->setType(Type::BOOL);
    node->setTypename("bool");
}

// IsNullNode --> ExprNode
void FourthPass::visit(IsNullNode* node) {
    
    AST::ExprNode* exprNode = node->getNode();
    if (exprNode != NULL) {
        exprNode->accept(this);

        if (exprNode->getType() != Type::CLASS && exprNode->getTypename() != "Object") {
            error(exprNode->getFilename(), exprNode->getLine(), exprNode->getColumn(), "this literal has type " + exprNode->getTypename() + ", but expected type was Object.");
        }
    }

    node->setType(Type::BOOL);
    node->setTypename("bool");
}

void FourthPass::visit(NewExprNode* node) {
    
    node->setType(Type::CLASS);
    node->setTypename(node->getTypeName()); //getTypeName is from NewExprNode, getTypename is from ExprNode

    // Check
    if (!isClassDefined(node->getTypename())) {
        error(node->getFilename(), node->getLine(), node->getColumn(), "Undefined type " + node->getTypeName());
    }
}

void FourthPass::visit(VariableExprNode* node) {

    /*
        The names of local variables (introduced by let ... in, see below), formal parameters of methods, self and class fields are expressions. They evaluate to the current value associated with the local variable, parameter or field, and have the corresponding type.
        
        The binding of an identifier references the innermost lexical scope that contains a declaration for that identifier, or to the field of the same name if there is no other declaration. A field can thus be hidden by a formal parameter or local variable, a formal parameter can be hidden by a local variable, and a local variable can be hidden by another local variable declaration with the same name within its scope.
        
        The exception to the previous rule is the identifier self, which is implicitly bound to the current object in every method, and cannot be hidden (it is a syntax error to declare a field, formal parameter or local variable named self). Note that self is only bound in method bodies, it is not in scope in field initializers.
    */
    
    if (node->getVariableName() == "self"){

        if (inField_) {
            error(node->getFilename(), node->getLine(), node->getColumn(), "Cannot use self in field initializer.");
        }

        node->setType(Type::CLASS);
        node->setTypename(currentClassName_);

    } else {

        // Check variable in let
        if (!currentLetVar_.empty()) {

            // if (currentLetVar_ == node->getVariableName() && node->getType() == currentTypeLetVar_ && node->getTypename() == currentLetTypename_) {
            if (currentLetVar_ == node->getVariableName()) {
                
                node->setType(currentTypeLetVar_);
                node->setTypename(currentLetTypename_);

                return;
            }
        }

        std::string currentClass = currentClassName_;
        std::string methodName = currentMethodName_;

        // loop walks up class hierarchy
        while (classInfoMap_.find(currentClass) != classInfoMap_.end()) {

            // check if currentClass in classMethodTypes_ --> does the class have methods?
            if (classMethodTypes_.find(currentClass) != classMethodTypes_.end()) {

                // check if methodName in classMethodTypes_ at currentClass --> does the method belong to this class?
                if (classMethodTypes_.at(currentClass).find(methodName) != classMethodTypes_.at(currentClass).end()) {

                    // check if currentClass in classMethodFormalsTypes_
                    auto classIt = classMethodFormalsTypes_.find(currentClass);
                    if (classIt != classMethodFormalsTypes_.end()) {

                        // check if methodName in classMethodFormalsTypes_ at currentClass
                        auto methodIt = classIt->second.find(methodName);
                        if (methodIt != classIt->second.end()) {

                            // check if formalIt (= Formal) from classMethodFormalsTypes_ at currentMethodName_ at currentClassName_
                            auto formalIt = methodIt->second.find(node->getVariableName());
                            if (formalIt != methodIt->second.end()) {

                                // check if it (= Type) in classMethodFormalsTypes_ at formalIt (= Formal) at currentMethodName_ at currentClassName_
                                const auto it = type_map_.find(formalIt->second);
                                if (it != type_map_.end()) {
                                    node->setType(it->second);
                                    node->setTypename(it->first);
                                } else {
                                    node->setType(Type::CLASS);
                                    node->setTypename(formalIt->second);
                                }

                                return;
                            } else {
                                
                                // check if currentClass in classFieldTypes_ --> does the class have fields?
                                auto classIt =  classFieldTypes_.find(currentClass);
                                if (classIt != classFieldTypes_.end()) {
                                    
                                    // check if fieldIt (= Field) in classFieldTypes_ at currentClass
                                    auto fieldIt = classIt->second.find(node->getVariableName());
                                    if (fieldIt != classIt->second.end()) {
                                        
                                        if (inField_) {
                                            error(node->getFilename(), node->getLine(), node->getColumn(), "Cannot use class fields in field initializer.");
                                        }

                                        // check if it (= Type) in classFieldTypes_ at fieldIt (= Field) at currentClass
                                        const auto it = type_map_.find(fieldIt->second);
                                        if (it != type_map_.end()) {
                                            node->setType(it->second);
                                            node->setTypename(it->first);
                                        } else {
                                            node->setType(Type::CLASS);
                                            node->setTypename(fieldIt->second);
                                        }
                                        return;
                                    }
                                }
                            }
                        } else {
                            
                            // check if currentClass in classFieldTypes_ --> does the class have fields?
                            auto classIt =  classFieldTypes_.find(currentClass);
                            if (classIt != classFieldTypes_.end()) {
                                
                                // check if fieldIt (= Field) in classFieldTypes_ at currentClass
                                auto fieldIt = classIt->second.find(node->getVariableName());
                                if (fieldIt != classIt->second.end()) {
                                    
                                    if (inField_) {
                                        error(node->getFilename(), node->getLine(), node->getColumn(), "Cannot use class fields in field initializer.");
                                    }

                                    // check if it (= Type) in classFieldTypes_ at fieldIt (= Field) at currentClass
                                    const auto it = type_map_.find(fieldIt->second);
                                    if (it != type_map_.end()) {
                                        node->setType(it->second);
                                        node->setTypename(it->first);
                                    } else {
                                        node->setType(Type::CLASS);
                                        node->setTypename(fieldIt->second);
                                    }

                                    return;
                                }
                            }
                        }
                    } else {
                        
                        // check if currentClass in classFieldTypes_ --> does the class have fields?
                        auto classIt =  classFieldTypes_.find(currentClass);
                        if (classIt != classFieldTypes_.end()) {
                            
                            // check if fieldIt (= Field) in classFieldTypes_ at currentClass
                            auto fieldIt = classIt->second.find(node->getVariableName());
                            if (fieldIt != classIt->second.end()) {
                                
                                if (inField_) {
                                    error(node->getFilename(), node->getLine(), node->getColumn(), "Cannot use class fields in field initializer.");
                                }

                                // check if it (= Type) in classFieldTypes_ at fieldIt (= Field) at currentClass
                                const auto it = type_map_.find(fieldIt->second);
                                if (it != type_map_.end()) {
                                    node->setType(it->second);
                                    node->setTypename(it->first);
                                } else {
                                    node->setType(Type::CLASS);
                                    node->setTypename(fieldIt->second);
                                }

                                return;
                            }
                        }
                    }
                } else {

                    // check if methodName in classMethodTypes_ at currentClass --> does the method belong to this class?: NO --> check fields

                    // check if currentClass in classFieldTypes_ --> does the class have fields?
                    auto classIt =  classFieldTypes_.find(currentClass);
                    if (classIt != classFieldTypes_.end()) {
                        
                        // check if fieldIt (= Field) in classFieldTypes_ at currentClass
                        auto fieldIt = classIt->second.find(node->getVariableName());
                        if (fieldIt != classIt->second.end()) {
                            
                            if (inField_) {
                                error(node->getFilename(), node->getLine(), node->getColumn(), "Cannot use class fields in field initializer.");
                            }

                            // check if it (= Type) in classFieldTypes_ at fieldIt (= Field) at currentClass
                            const auto it = type_map_.find(fieldIt->second);
                            if (it != type_map_.end()) {
                                node->setType(it->second);
                                node->setTypename(it->first);
                            } else {
                                node->setType(Type::CLASS);
                                node->setTypename(fieldIt->second);
                            }
                            return;
                        }
                    }
                }
            } else {
                // check if currentClass in classMethodTypes_ --> does the class have methods?: NO --> check fields

                // check if currentClass in classFieldTypes_ --> does the class have fields?
                auto classIt =  classFieldTypes_.find(currentClass);
                if (classIt != classFieldTypes_.end()) {
                    
                    // check if fieldIt (= Field) in classFieldTypes_ at currentClass
                    auto fieldIt = classIt->second.find(node->getVariableName());
                    if (fieldIt != classIt->second.end()) {
                        
                        if (inField_) {
                            error(node->getFilename(), node->getLine(), node->getColumn(), "Cannot use class fields in field initializer.");
                        }

                        // check if it (= Type) in classFieldTypes_ at fieldIt (= Field) at currentClass
                        const auto it = type_map_.find(fieldIt->second);
                        if (it != type_map_.end()) {
                            node->setType(it->second);
                            node->setTypename(it->first);
                        } else {
                            node->setType(Type::CLASS);
                            node->setTypename(fieldIt->second);
                        }
                        return;
                    }
                }
            }
            currentClass = classInfoMap_.at(currentClass).parentClassName;
        }
    }

    if (type_map_.find(node->getTypename()) == type_map_.end()) {
        if (node->getType() != AST::Type::CLASS) {
            error(node->getFilename(), node->getLine(), node->getColumn(), "Use of unbound variable " + node->getVariableName() + ".");
        }
    }
}

void FourthPass::visit(UnitExprNode* node) {
    node->setType(Type::UNIT);
    node->setTypename("unit");
}

// ParExprNode --> ExprNode
void FourthPass::visit(ParExprNode* node) {
    
    AST::ExprNode* exprNode = node->getExpr();
    if (exprNode != NULL) {
        exprNode->accept(this);
    }

    if (exprNode->getType() == Type::CLASS) {
        node->setType(exprNode->getType());
        node->setTypename(exprNode->getTypename());
    } else {
        node->setType(exprNode->getType());
        node->setTypename(exprNode->getTypename());
    }
}

// BlockExprNode --> BlockNode
void FourthPass::visit(BlockExprNode* node) {
    
    AST::BlockNode* blockNode = node->getBlock();
    if (blockNode != NULL) {
        blockNode->accept(this);
        node->setType(blockNode->getType());
        node->setTypename(blockNode->getTypename());
    }
}

// BlockNode --> ExprNode
void FourthPass::visit(BlockNode* node) {
    
    // Visit each expr node in block
    std::vector<ExprNode*>& exprNodes = node->getExpr();
    std::reverse(exprNodes.begin(), exprNodes.end());
    for (auto& expr_node : exprNodes) {
        expr_node->accept(this);
        node->setType(expr_node->getType());
        node->setTypename(expr_node->getTypename());
    }
}

// ArgsNode --> ExprNode
void FourthPass::visit(ArgsNode* node) {
    
    // Visit each expr node in args
    std::vector<ExprNode*>& exprNodes = node->getExprs();
    std::reverse(exprNodes.begin(), exprNodes.end());
    for (auto& expr_node : node->getExprs()) {
        expr_node->accept(this);
    }
}

// FieldNode --> TypeNode and ExprNode
void FourthPass::visit(FieldNode* node) {
    
    inField_ = true;

    AST::TypeNode* typeNode = node->getType();
    // if (typeNode != NULL) {
    //     typeNode->accept(this);
    // }

    /* If a field has the optional initializer
    * The type of the initializer must conform to the (static) type of
    * the field
    */
    AST::ExprNode* initExprNode = node->getInitExpr();
    if (initExprNode != NULL) {
        
        initExprNode->accept(this);

        // Error if not the same type
        if (typeNode->getType() != initExprNode->getType()) {
            error(initExprNode->getFilename(), initExprNode->getLine(), initExprNode->getColumn(),
                "The type of the initializer must conform to the type of the field ");     
        }

        // Error if not the same class
        if (typeNode->getType() == Type::CLASS && (typeNode->getTypeStr() != initExprNode->getTypename())) {

            std::unordered_set<std::string> ancestors;

            // Stock all ancestor of class
            std::string currentClass = initExprNode->getTypename();
            while (currentClass != "Object") {
                ancestors.insert(currentClass);
                currentClass = classInfoMap_.at(currentClass).parentClassName;
            }
            ancestors.insert("Object");

            if (ancestors.find(typeNode->getTypeStr()) == ancestors.end()) {
                error(initExprNode->getFilename(), initExprNode->getLine(), initExprNode->getColumn(),
                    "The type of the initializer must conform to the type of the field ");
            }
        }
    }

    inField_ = false;
}

// MethodNode --> FormalsNode and BlockNode
void FourthPass::visit(MethodNode* node) {
    currentMethodName_ = node->getName();

    // reset currentLetVar_  and currentLetTypename_ 
    currentLetVar_ = "";
    currentLetTypename_ = "";

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

    if (currentClassName_ != "Object" && (retTypeNode->getType() !=  blockNode->getType())) {

        error(node->getFilename(), node->getLine(), node->getColumn(), "The type of the method body must conform to its declared return type.");
    }

    // Error if not the same class
    if (currentClassName_ != "Object" && (retTypeNode->getType() == blockNode->getType()) && retTypeNode->getType() == Type::CLASS && (retTypeNode->getTypeStr() != blockNode->getTypename())) {

        std::unordered_set<std::string> ancestors;

        // Stock all ancestor of class
        std::string currentClass = blockNode->getTypename();
        while (currentClass != "Object") {
            ancestors.insert(currentClass);
            currentClass = classInfoMap_.at(currentClass).parentClassName;
        }
        ancestors.insert("Object");

        if (ancestors.find(retTypeNode->getTypeStr()) == ancestors.end()) {
            error(node->getFilename(), node->getLine(), node->getColumn(),
                "The type of the method body must conform to its declared return type.");
        }
    }
}

// ClassBodyNode --> MethodNode and FieldNode
void FourthPass::visit(ClassBodyNode* node) {
    
    // Visit each field and method node in class body
    for (auto& field_node : node->getFields()) {
        field_node->accept(this);
    }

    for (auto& method_node : node->getMethods()) {
        method_node->accept(this);
    }
}

// CallExprNode --> ExprNode and ArgsNode*
void FourthPass::visit(CallExprNode* node) {

    /*
        <expr_0>.<id>(<expr_1>, ..., <expr_n>)
        To typecheck the dispatch, assuming the static type of <expr_0> is P, the compiler will check
        that class P has (or inherits) a method <id> with <n> formal parameters (<n> can be zero), such
        that the static type of the i-th actual argument <expr_i> conforms to the type of the i-th formal
        parameter.
        */
        /*
        To evaluate the dispatch, <expr_0> is evaluated first. Arguments <expr_1>, . . . , <expr_n> are
        then evaluated from left to right. Finally, assuming <expr_0> has dynamic type C, the method
        <id> of class C is invoked with self bound to the value of <expr_0> and its formal parameters
        bound to the values of the actual arguments <expr_1>, . . . , <expr_n>. The value of the expression
        is the value returned by the method invocation.
    */

    AST::ExprNode* objExprNode = node->getObjExpr();
    if (objExprNode != NULL) {
        objExprNode->accept(this);
    }

    std::string currentClass = objExprNode->getTypename();
    std::string methodName = node->getMethodName();

    // is method defined ?
    if (!isMethodDefined(currentClass, methodName)) {
        error(node->getFilename(), node->getLine(), node->getColumn(), "Undefined method " + methodName);
    }

    AST::ArgsNode* exprListNode = node->getExprList();
    if (exprListNode != NULL) {
        exprListNode->accept(this);
    }

    if (objExprNode->getTypename() == "self") {
        node->setType(Type::CLASS);
        node->setTypename("self");
    } else {
        
        // loop walks up class hierarchy
        while (classInfoMap_.find(currentClass) != classInfoMap_.end()) {

            
            // check if currentClass in classMethodTypes_
            if (classMethodTypes_.find(currentClass) != classMethodTypes_.end()) {

                // check if methodName in classMethodTypes_ at currentClass
                if (classMethodTypes_.at(currentClass).find(methodName) != classMethodTypes_.at(currentClass).end()) {

                    
                    // check if currentClass in classMethodFormalsTypes_
                    auto classIt = classMethodFormalsTypes_.find(currentClass);
                    if (classIt != classMethodFormalsTypes_.end()) {

                        
                        // check if methodName in classMethodFormalsTypes_ at currentClass
                        auto methodIt = classIt->second.find(methodName);
                        if (methodIt != classIt->second.end()) {

                            
                            // check if same number of formals and args
                            auto methodIt = classMethodFormalsTypes_.at(currentClass).find(methodName);
                            if (methodIt->second.size() != exprListNode->getExprs().size()) {
                                error(node->getFilename(), node->getLine(), node->getColumn(), "arguments missing");
                            } else {

                                // Iterate through the vector using a for loop
                                int i = 0;
                                for (auto formal : methodIt->second) {

                                    const auto it = type_map_.find(formal.second);
                                    if (it != type_map_.end()) {

                                        if (it->second != exprListNode->getExprs()[i]->getType()) {
                                            error(exprListNode->getExprs()[i]->getFilename(), exprListNode->getExprs()[i]->getLine(), exprListNode->getExprs()[i]->getColumn(), "arg type not corresponding to definition of method");
                                        }
                                    } else {

                                        // is this check needed ?? (It is the one causing the segfault) check for formals with type CLASS

                                        // class type
                                        // if (node->getFormals()->getFormals()[i]->getType()->getType() == Type::CLASS && (formal.second != exprListNode->getExprs()[i]->getTypename())) {
                                        //     // debug
                                        //     std::string myString19 = "debug debut if class type CallExprNode";
                                        //     std::cout << myString19 << std::endl;

                                        //     error(exprListNode->getExprs()[i]->getFilename(),
                                        //         exprListNode->getExprs()[i]->getLine(),
                                        //         exprListNode->getExprs()[i]->getColumn(),
                                        //         "arg type not corresponding to definition of method");
                                        // }
                                        
                                    }
                                    
                                    i++;
                                }
                            }
                        } 
                    } 

                    // retrieve current method from current class
                    // class
                    auto classMIt =  classMethodTypes_.find(currentClass);
                    if (classMIt != classMethodTypes_.end()) {

                        // method
                        auto methodIt = classMIt->second.find(methodName);
                        if (methodIt != classMIt->second.end()) {

                            // set type
                            const auto it = type_map_.find(methodIt->second);
                            if (it != type_map_.end()) {
                                node->setType(it->second);
                                node->setTypename(it->first);
                            } else {
                                node->setType(Type::CLASS);
                                node->setTypename(methodIt->second);
                            }
                            break;
                        }
                    }
                } 
            }
            currentClass = classInfoMap_.at(currentClass).parentClassName;
        }
    }
}

void FourthPass::visit(IntegerLiteralNode* node) {
    
    node->setType(Type::INT32);
    node->setTypename("int32");
}

void FourthPass::visit(LiteralNode* node) {
    
    node->setType(Type::STRING);
    node->setTypename("string");
}

void FourthPass::visit(BooleanLiteralNode* node) {
    
    node->setType(Type::BOOL);
    node->setTypename("bool");
}

bool FourthPass::isClassDefined(const std::string& className) {
    return classInfoMap_.find(className) != classInfoMap_.end();
}

bool FourthPass::isTypeDefined(const AST::Type& type) {
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

bool FourthPass::isMethodDefined(const std::string& className, const std::string& methodName) {
    std::string currentClass = className;

    // loop walks up class hierarchy
    while (classInfoMap_.find(currentClass) != classInfoMap_.end()) {

        // check if currentClass in classMethodTypes_
        if (classMethodTypes_.find(currentClass) != classMethodTypes_.end()) {

            // check if methodName in classMethodTypes_ at currentClass
            if (classMethodTypes_.at(currentClass).find(methodName) != classMethodTypes_.at(currentClass).end()) {
                return true;
            } 
        }
        currentClass = classInfoMap_.at(currentClass).parentClassName;
    }
    return false;
}

// Implement the accept methods for each AST node
void ProgramNode::accept(Visitor* visitor) { visitor->visit(this); }
void TypeNode::accept(Visitor* visitor) { visitor->visit(this); }
void FormalNode::accept(Visitor* visitor) { visitor->visit(this); }
void FormalsNode::accept(Visitor* visitor) { visitor->visit(this); }
void ExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void IfThenElseExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void WhileDoExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void LetExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void AssignExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void BinaryExprNode::accept(Visitor* visitor) { visitor->visit(this); }

void AndNode::accept(Visitor* visitor) { visitor->visit(this); }
void EqualNode::accept(Visitor* visitor) { visitor->visit(this); }
void LowerNode::accept(Visitor* visitor) { visitor->visit(this); }
void LowerEqualNode::accept(Visitor* visitor) { visitor->visit(this); }
void PlusNode::accept(Visitor* visitor) { visitor->visit(this); }
void MinusNode::accept(Visitor* visitor) { visitor->visit(this); }
void TimesNode::accept(Visitor* visitor) { visitor->visit(this); }
void DivNode::accept(Visitor* visitor) { visitor->visit(this); }
void PowNode::accept(Visitor* visitor) { visitor->visit(this); }

void UnaryExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void UnaryMinusNode::accept(Visitor* visitor) { visitor->visit(this); }
void NotNode::accept(Visitor* visitor) { visitor->visit(this); }
void IsNullNode::accept(Visitor* visitor) { visitor->visit(this); }

void NewExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void VariableExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void UnitExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void ParExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void BlockNode::accept(Visitor* visitor) { visitor->visit(this); }
void BlockExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void ArgsNode::accept(Visitor* visitor) { visitor->visit(this); }
void FieldNode::accept(Visitor* visitor) { visitor->visit(this); }
void MethodNode::accept(Visitor* visitor) { visitor->visit(this); }
void ClassBodyNode::accept(Visitor* visitor) { visitor->visit(this); }
void ClassNode::accept(Visitor* visitor) { visitor->visit(this); }
void CallExprNode::accept(Visitor* visitor) { visitor->visit(this); }
void IntegerLiteralNode::accept(Visitor* visitor) { visitor->visit(this); }
void LiteralNode::accept(Visitor* visitor) { visitor->visit(this); }
void BooleanLiteralNode::accept(Visitor* visitor) { visitor->visit(this); }
