#include "ast.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace AST;

bool checkMode = false;

static const std::unordered_map<Type, std::string> type_map = {
    {Type::INT32,"int32"},
    {Type::BOOL,"bool"},
    {Type::STRING,"string"},
    {Type::UNIT,"unit"}
};

/*
 *   Helpers
 */

// Joins elements of a vector with a delimiter.
std::string join(const std::vector<std::string>& elements, const std::string& delimiter = ", ") {
    std::string result;
    bool first = true;

    for (const auto& element : elements) {
        if (!first) {
            result += delimiter;
        }
        result += element;
        first = false;
    }

    return result;
}

// Returns optional expression string if the expression is not null.
std::string optional_expr(const ExprNode* expr) {
    return expr ? ", " + expr->evaluate() : "";
}

// Converts an operator to its corresponding string representation.
std::string operatorToString(Operator op) {
    switch (op) {
        case Operator::AND:
            return "and";
        case Operator::EQUAL:
            return "=";
        case Operator::LOWER:
            return "<";
        case Operator::LOWER_EQUAL:
            return "<=";
        case Operator::PLUS:
            return "+";
        case Operator::MINUS:
            return "-";
        case Operator::TIMES:
            return "*";
        case Operator::DIV:
            return "/";
        case Operator::POW:
            return "^";
        case Operator::NOT:
            return "not";
        case Operator::ISNULL:
            return "isnull";
        default:
            return "unknown";
    }
}

/*
 *   Program
 */

// constructor
ProgramNode::ProgramNode(std::vector<ClassNode*> classes, const std::string& filename, int line, int column)
: filename(filename), line(line), column(column), classes_(classes) {
    std::string* type = new std::string("Object");
    AST::TypeNode* typeNodeObject = new AST::TypeNode(type, filename, 0, 0);

    // print(s : string) : Object
    std::string* name = new std::string("s");
    type = new std::string("string");
    AST::TypeNode* typeNode = new AST::TypeNode(type, filename, 0, 0);
    AST::FormalNode* formal = new AST::FormalNode(name, typeNode);
    std::vector<AST::FormalNode*> formalVectPrint;
    formalVectPrint.push_back(formal);
    AST::FormalsNode* formalsPrint = new AST::FormalsNode(formalVectPrint);

    name = new std::string("print");
    std::vector<AST::ExprNode*> expr_list_print;
    AST::BlockNode* blockNodePrint = new AST::BlockNode(expr_list_print);
    blockNodePrint->setType(Type::CLASS);
    blockNodePrint->setTypename("Object");
    AST::MethodNode* print = new AST::MethodNode(name, formalsPrint, typeNodeObject, blockNodePrint, filename, 0, 0);

    // printBool(b : bool) : Object
    name = new std::string("b");
    type = new std::string("bool");
    typeNode = new AST::TypeNode(type, filename, 0, 0);
    formal = new AST::FormalNode(name, typeNode);
    std::vector<AST::FormalNode*> formalVectPrintBool;
    formalVectPrintBool.push_back(formal);
    AST::FormalsNode* formalsPrintBool = new AST::FormalsNode(formalVectPrintBool);

    name = new std::string("printBool");
    std::vector<AST::ExprNode*> expr_list_printBool;
    AST::BlockNode* blockNodePrintBool = new AST::BlockNode(expr_list_printBool);
    blockNodePrintBool->setType(Type::CLASS);
    blockNodePrintBool->setTypename("Object");
    AST::MethodNode* printBool = new AST::MethodNode(name, formalsPrintBool, typeNodeObject, blockNodePrintBool, filename, 0, 0);

    // printInt32(i : int32) : Object
    name = new std::string("i");
    type = new std::string("int32");
    typeNode = new AST::TypeNode(type, filename, 0, 0);
    formal = new AST::FormalNode(name, typeNode);
    std::vector<AST::FormalNode*> formalVectPrintInt32;
    formalVectPrintInt32.push_back(formal);
    AST::FormalsNode* formalsPrintInt32 = new AST::FormalsNode(formalVectPrintInt32);

    name = new std::string("printInt32");
    std::vector<AST::ExprNode*> expr_list_printInt32;
    AST::BlockNode* blockNodePrintInt32 = new AST::BlockNode(expr_list_printInt32);
    blockNodePrintInt32->setType(Type::CLASS);
    blockNodePrintInt32->setTypename("Object");
    AST::MethodNode* printInt32 = new AST::MethodNode(name, formalsPrintInt32, typeNodeObject, blockNodePrintInt32, filename, 0, 0);

    // inputLine() : string
    std::vector<AST::FormalNode*> formalVectInputLine;
    AST::FormalsNode* formalsInputLine = new AST::FormalsNode(formalVectInputLine);
    type = new std::string("string");
    typeNode = new AST::TypeNode(type, filename, 0, 0);

    name = new std::string("inputLine");
    std::vector<AST::ExprNode*> expr_list_inputLine;
    AST::BlockNode* blockNodeInputLine = new AST::BlockNode(expr_list_inputLine);
    blockNodeInputLine->setType(Type::STRING);
    blockNodeInputLine->setTypename("string");
    AST::MethodNode* inputLine = new AST::MethodNode(name, formalsInputLine, typeNode, blockNodeInputLine, filename, 0, 0);

    // inputBool() : bool
    std::vector<AST::FormalNode*> formalVectInputBool;
    AST::FormalsNode* formalsInputBool = new AST::FormalsNode(formalVectInputBool);
    type = new std::string("bool");
    typeNode = new AST::TypeNode(type, filename, 0, 0);

    name = new std::string("inputBool");
    std::vector<AST::ExprNode*> expr_list_inputBool;
    AST::BlockNode* blockNodeInputBool = new AST::BlockNode(expr_list_inputBool);
    blockNodeInputBool->setType(Type::BOOL);
    blockNodeInputBool->setTypename("bool");
    AST::MethodNode* inputBool = new AST::MethodNode(name, formalsInputBool, typeNode, blockNodeInputBool, filename, 0, 0);

    // inputInt32() : int32
    std::vector<AST::FormalNode*> formalVectInputInt32;
    AST::FormalsNode* formalsInputInt32 = new AST::FormalsNode(formalVectInputInt32);
    type = new std::string("int32");
    typeNode = new AST::TypeNode(type, filename, 0, 0);

    name = new std::string("inputInt32");
    std::vector<AST::ExprNode*> expr_list_inputInt32;
    AST::BlockNode* blockNodeInputInt32 = new AST::BlockNode(expr_list_inputInt32);
    blockNodeInputInt32->setType(Type::INT32);
    blockNodeInputInt32->setTypename("int32");
    AST::MethodNode* inputInt32 = new AST::MethodNode(name, formalsInputInt32, typeNode, blockNodeInputInt32, filename, 0, 0);

    // add Object class
    name = new std::string("Object");
    std::string* parent = new std::string("");
    std::vector<AST::FieldNode*> fields;
    std::vector<AST::MethodNode*> methods;
    // Adding object prototypes to methods vector
    methods.push_back(print);
    methods.push_back(printBool);
    methods.push_back(printInt32);
    methods.push_back(inputLine);
    methods.push_back(inputBool);
    methods.push_back(inputInt32);
    AST::ClassBodyNode* class_body = new AST::ClassBodyNode(fields, methods);
    AST::ClassNode* objectCls = new AST::ClassNode(name, parent, class_body, filename, 0, 0);

    // add Object class to the classes_ vector at 1st position
    classes_.insert(classes_.begin(), objectCls);
} 

std::string ProgramNode::evaluate() const {
    
    std::vector<std::string> cls_strings;
    for (const auto& cls : classes_) {
        if (cls->getName() == "Object")
            continue;
        cls_strings.push_back(cls->evaluate());
    }
    std::reverse(cls_strings.begin(), cls_strings.end());
    
    return "[" + join(cls_strings) + "]";
}

/*
 *   Class
 */

// constructor
ClassNode::ClassNode(std::string* name, std::string* parent, ClassBodyNode* class_body,
    const std::string& filename, int line, int column)
: filename(filename),line(line), column(column), name_(std::move(*name)),
parent_(std::move(*parent)), class_body_(class_body) {}

// evaluate
std::string ClassNode::evaluate() const {

    if (parent_.empty())
        return "Class(" + name_ + ", Object, " + class_body_->evaluate() + ")";
    
    return "Class(" + name_ + ", " + parent_ + ", " + class_body_->evaluate() + ")";
}
// constructor
ClassBodyNode::ClassBodyNode(std::vector<FieldNode*> fields, std::vector<MethodNode*> methods)
: fields_(fields), methods_(methods) {}

// evaluate
std::string ClassBodyNode::evaluate() const {
    
    std::vector<std::string> field_strings;
    for (const auto& field : fields_) {
        field_strings.push_back(field->evaluate());
    }
    std::reverse(field_strings.begin(), field_strings.end());

    std::vector<std::string> method_strings;
    for (const auto& method : methods_) {
        method_strings.push_back(method->evaluate());
    }
    std::reverse(method_strings.begin(), method_strings.end());

    return "[" + join(field_strings) + "], [" + join(method_strings) + "]";
}

/*
 *   Field
 */

// constructor
FieldNode::FieldNode(std::string* name, TypeNode* type, ExprNode* init_expr,
const std::string& filename, int line, int column)
: filename(filename), line(line), column(column), name_(std::move(*name)),
type_(type), init_expr_(init_expr) {}

// evaluate
std::string FieldNode::evaluate() const {
    
    return "Field(" + name_ + ", " + type_->evaluate() + optional_expr(init_expr_) + ")";
}

/*
 *   Method
 */

// constructor
MethodNode::MethodNode(std::string* name, FormalsNode* formals, TypeNode* ret_type,
BlockNode* block, const std::string& filename, int line, int column)
: filename(filename), line(line), column(column), name_(std::move(*name)),
formals_(formals), ret_type_(ret_type), block_(block) {}

// evaluate
std::string MethodNode::evaluate() const {
    
    std::string formalsString = formals_ ? formals_->evaluate() : "[]";

    std::string blockString = block_ ? block_->evaluate() : "[]";

    std::string type = "";
    if (checkMode) {
        
        std::string typestr = "";
        if (!block_->getExpr().empty()) {
            Type typet = block_->getExpr().back()->getType();

            const auto it = type_map.find(typet);

            if (it != type_map.end()) {
                typestr = it->second;
            } else {
                typestr = block_->getExpr().back()->getTypename();
            }
        } else {

            typestr = block_->getTypename();
        }
        
        type = " : " + typestr; 
    }

    return "Method(" + name_ + ", " + formalsString + ", " + ret_type_->evaluate() + ", " + blockString + type + ")" ;

}

/*
 *   Types
 */

// constructor
TypeNode::TypeNode(std::string* type, const std::string& filename, int line, int column)
: filename(filename), line(line), column(column), typeStr_(std::move(*type)) {}

// evaluate
std::string TypeNode::evaluate() const {
    
    return typeStr_;
}

/*
 *   Formals
 */

// constructor
FormalsNode::FormalsNode(std::vector<FormalNode*> formals) : formals_(formals) {}

// evaluate
std::string FormalsNode::evaluate() const {
    
    std::vector<std::string> formal_strings;
    for (const auto& formal : formals_) {
        formal_strings.push_back(formal->evaluate());
    }
    std::reverse(formal_strings.begin(), formal_strings.end());

    return "[" + join(formal_strings) + "]";
}

// constructor
FormalNode::FormalNode(std::string* name, TypeNode* type) : name_(std::move(*name)), type_(type) {}

// evaluate
std::string FormalNode::evaluate() const {
    
    return name_ + " : " + type_->evaluate();
}

/*
 *   Blocks
 */

// constructor
BlockNode::BlockNode(std::vector<ExprNode*> expr_list) : expr_list_(expr_list) {}

// evaluate
std::string BlockNode::evaluate() const {
    
    std::vector<std::string> expr_strings;
    for (const auto& expr : expr_list_) {
        expr_strings.push_back(expr->evaluate());
    }
    if (!checkMode) {
        std::reverse(expr_strings.begin(), expr_strings.end());
    }

    return "[" + join(expr_strings) + "]";
}


/*
 *   Expressions
 */

// constructor
BlockExprNode::BlockExprNode(BlockNode* block, const std::string& filename, int line, int column)
: ExprNode(filename, line, column), block_(block) {}
   
// evaluate         
std::string BlockExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        Type typet = getType();
        std::string typestr = "";
        const auto it = type_map.find(typet);
        if (it != type_map.end()) {
            typestr = it->second;
        } else {
            typestr = getTypename();
        }
        
        type = " : " + typestr; 
    }
    return block_->evaluate() + type;
}

// constructor
IfThenElseExprNode::IfThenElseExprNode(ExprNode* cond_expr, ExprNode* then_expr,
ExprNode* else_expr, const std::string& filename, int line, int column)
: ExprNode(filename, line, column), cond_expr_(cond_expr), then_expr_(then_expr), else_expr_(else_expr){}

// evaluate
std::string IfThenElseExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        Type typet = getType();
        std::string typestr = "";
        const auto it = type_map.find(typet);
        if (it != type_map.end()) {
            typestr = it->second;
        } else {
            typestr = getTypename();
        }
        
        type = " : " + typestr; 
    }
    return "If(" + cond_expr_->evaluate() + ", " + then_expr_->evaluate() + optional_expr(else_expr_) + ")" + type;
}

// constructor
WhileDoExprNode::WhileDoExprNode(ExprNode* cond_expr, ExprNode* body_expr,
const std::string& filename, int line, int column)
: ExprNode(filename, line, column), cond_expr_(cond_expr), body_expr_(body_expr) {}

// evaluate
std::string WhileDoExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        Type typet = getType();
        std::string typestr = "";
        const auto it = type_map.find(typet);
        if (it != type_map.end()) {
            typestr = it->second;
        } else {
            typestr = getTypename();
        }
        
        type = " : " + typestr; 
    }
    return "While(" + cond_expr_->evaluate() + ", " + body_expr_->evaluate() + ")" + type;
}

// constructor
LetExprNode::LetExprNode(std::string* name, TypeNode* type_node, ExprNode* init_expr,
ExprNode* scope_expr, const std::string& filename, int line, int column)
: ExprNode(filename, line, column), name_(std::move(*name)),
type_node_(type_node), init_expr_(init_expr), scope_expr_(scope_expr)  {}

// evaluate
std::string LetExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        Type typet = getType();
        std::string typestr = "";
        const auto it = type_map.find(typet);
        if (it != type_map.end()) {
            typestr = it->second;
        } else {
            typestr = getTypename();
        }
        
        type = " : " + typestr; 
    }
    return "Let(" + name_ + ", " + type_node_->evaluate() + optional_expr(init_expr_) + ", " + scope_expr_->evaluate() + ")" + type;
}

// constructor
AssignExprNode::AssignExprNode(std::string* name_str, ExprNode* name, ExprNode* expr,
const std::string& filename, int line, int column)
: ExprNode(filename, line, column), name_str_(std::move(*name_str)), name_(name), expr_(expr) {}

// evaluate
std::string AssignExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        Type typet = getType();
        std::string typestr = "";
        const auto it = type_map.find(typet);
        if (it != type_map.end()) {
            typestr = it->second;
        } else {
            typestr = getTypename();
        }
         
        type = " : " + typestr; 
    }
    return "Assign(" + name_str_ + ", " + expr_->evaluate() + ")" + type;
}

/*
 *   BinaryExprNode
 */

// constructor
BinaryExprNode::BinaryExprNode(Operator op, ExprNode* left_expr, ExprNode* right_expr,
const std::string& filename, int line, int column)
: ExprNode(filename, line, column), left_expr_(left_expr), right_expr_(right_expr), op_(op) {}

// evaluate
std::string BinaryExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        Type typet = getType();
        std::string typestr = "";
        const auto it = type_map.find(typet);
        if (it != type_map.end()) {
            typestr = it->second;
        } else {
            typestr = getTypename();
        }
        
        type = " : " + typestr; 
    }
    return "BinOp(" + operatorToString(op_) + ", " + left_expr_->evaluate() + ", " + right_expr_->evaluate() + ")" + type;
}

AndNode::AndNode(ExprNode* left_expr, ExprNode* right_expr,
const std::string& filename, int line, int column)
: BinaryExprNode(Operator::AND, left_expr, right_expr, filename, line, column) {}

EqualNode::EqualNode(ExprNode* left_expr, ExprNode* right_expr,
const std::string& filename, int line, int column)
: BinaryExprNode(Operator::EQUAL, left_expr, right_expr, filename, line, column) {}

LowerNode::LowerNode(ExprNode* left_expr, ExprNode* right_expr,
const std::string& filename, int line, int column)
:  BinaryExprNode(Operator::LOWER, left_expr, right_expr, filename, line, column) {}

LowerEqualNode::LowerEqualNode(ExprNode* left_expr, ExprNode* right_expr,
const std::string& filename, int line, int column)
: BinaryExprNode(Operator::LOWER_EQUAL, left_expr, right_expr, filename, line, column) {}

PlusNode::PlusNode(ExprNode* left_expr, ExprNode* right_expr,
const std::string& filename, int line, int column)
: BinaryExprNode(Operator::PLUS, left_expr, right_expr, filename, line, column) {}

MinusNode::MinusNode(ExprNode* left_expr, ExprNode* right_expr,
const std::string& filename, int line, int column)
: BinaryExprNode(Operator::MINUS, left_expr, right_expr, filename, line, column) {}

TimesNode::TimesNode(ExprNode* left_expr, ExprNode* right_expr,
const std::string& filename, int line, int column)
: BinaryExprNode(Operator::TIMES, left_expr, right_expr, filename, line, column) {}

DivNode::DivNode(ExprNode* left_expr, ExprNode* right_expr,
const std::string& filename, int line, int column)
: BinaryExprNode(Operator::DIV, left_expr, right_expr, filename, line, column) {}

PowNode::PowNode(ExprNode* left_expr, ExprNode* right_expr,
const std::string& filename, int line, int column)
: BinaryExprNode(Operator::POW, left_expr, right_expr, filename, line, column) {}

/*
 *   UnaryExprNode
 */

// constructor
UnaryExprNode::UnaryExprNode(Operator op, ExprNode* expr,
const std::string& filename, int line, int column)
: ExprNode(filename, line, column), expr_(expr), op_(op) {}

// evaluate
std::string UnaryExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        std::string typestr = "bool";
         
        type = " : " + typestr; 
    }
    return "UnOp(" + operatorToString(op_) + ", " + expr_->evaluate() + ")" + type;
}

UnaryMinusNode::UnaryMinusNode(ExprNode* expr,
const std::string& filename, int line, int column)
: UnaryExprNode(Operator::MINUS, expr, filename, line, column) {}
// evaluate
std::string UnaryMinusNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        std::string typestr = "int32";
         
        type = " : " + typestr; 
    }
    return "UnOp(" + operatorToString(op_) + ", " + expr_->evaluate() + ")" + type;
}

NotNode::NotNode(ExprNode* expr, const std::string& filename,int line, int column)
: UnaryExprNode(Operator::NOT, expr, filename, line, column) {}
// evaluate
std::string NotNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        std::string typestr = "bool";
         
        type = " : " + typestr; 
    }
    return "UnOp(" + operatorToString(op_) + ", " + expr_->evaluate() + ")" + type;
}
IsNullNode::IsNullNode(ExprNode* expr, const std::string& filename, int line, int column)
: UnaryExprNode(Operator::ISNULL, expr, filename, line, column) {}
// evaluate
std::string IsNullNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        Type typet = getType();
        std::string typestr = "";
        const auto it = type_map.find(typet);
        if (it != type_map.end()) {
            typestr = it->second;
        } else {
            typestr = getTypename();
        }
        
        type = " : " + typestr; 
    }
    return "UnOp(" + operatorToString(op_) + ", " + expr_->evaluate() + ")" + type;
}
/*
 *   Call
 */

// constructor
CallExprNode::CallExprNode(ExprNode* obj_expr, std::string* method_name, ArgsNode* expr_list,
const std::string& filename, int line, int column)
: ExprNode(filename, line, column), method_name_(std::move(*method_name)), obj_expr_(obj_expr), expr_list_(expr_list) {}

// evaluate
std::string CallExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        Type typet = getType();
        std::string typestr = "";
        const auto it = type_map.find(typet);
        if (it != type_map.end()) {
            typestr = it->second;
        } else {
            typestr = getTypename();
        }
        
        type = " : " + typestr; 
    }
    return "Call(" + obj_expr_->evaluate() + ", " + method_name_ + ", " + expr_list_->evaluate() + ")"  + type;
}

/*
 *   New
 */

// constructor
NewExprNode::NewExprNode(std::string* type_name, const std::string& filename, int line, int column)
: ExprNode(filename, line, column), type_name_(std::move(*type_name)) {}

// evaluate
std::string NewExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        Type typet = getType();
        std::string typestr = "";
        const auto it = type_map.find(typet);
        if (it != type_map.end()) {
            typestr = it->second;
        } else {
            typestr = getTypename();
        }
           
        type = " : " + typestr; 
    }
    return "New(" + type_name_ + ")"  + type;
}

/*
 *   Variable names
 */

// constructor
VariableExprNode::VariableExprNode(std::string* variable_name, const std::string& filename, int line, int column)
: ExprNode(filename, line, column), variable_name_(std::move(*variable_name)) {}

// evaluate
std::string VariableExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) {
        Type typet = getType();
        std::string typestr = "";
        const auto it = type_map.find(typet);
        if (it != type_map.end()) {
            typestr = it->second;
        } else {
            typestr = getTypename();
        }
          
        type = " : " + typestr; 
    }
    return variable_name_ + type;
}

/*
 *   Unit
 */

// constructor
UnitExprNode::UnitExprNode(const std::string& filename, int line, int column)
: ExprNode(filename, line, column){}

// evaluate
std::string UnitExprNode::evaluate() const {
    std::string type = "";
    if (checkMode) { type = " : unit"; }
    return "()" + type;
}

// constructor
ParExprNode::ParExprNode(ExprNode* expr, const std::string& filename, int line, int column)
: ExprNode(filename, line, column), expr_(expr) {}

// evaluate
std::string ParExprNode::evaluate() const {
    return expr_->evaluate();
}

// constructor
ArgsNode::ArgsNode(std::vector<ExprNode*> exprs) : exprs_(exprs) {}

// evaluate
std::string ArgsNode::evaluate() const {
        
    std::vector<std::string> arg_strings;
    for (const auto& expr : exprs_) {
        arg_strings.push_back(expr->evaluate());
    }
    if (!checkMode) {
        std::reverse(arg_strings.begin(), arg_strings.end());
    }
    return "[" + join(arg_strings) + "]";
}

// constructor
IntegerLiteralNode::IntegerLiteralNode(std::string* integerliteral, const std::string& filename, int line, int column)
: ExprNode(filename, line, column), integerliteral_(std::move(*integerliteral)) {}

// evaluate
std::string IntegerLiteralNode::evaluate() const {
    std::string type = "";
    if (checkMode) { type = " : int32"; }
    return integerliteral_ + type;
}

// constructor
LiteralNode::LiteralNode(std::string* literal, const std::string& filename, int line, int column)
: ExprNode(filename, line, column), literal_(std::move(*literal)) {}

// evaluate
std::string LiteralNode::evaluate() const {
    std::string type = "";
    if (checkMode) { type = " : string"; }
    return literal_ + type;
}

// constructor
BooleanLiteralNode::BooleanLiteralNode(bool literal, const std::string& filename, int line, int column)
: ExprNode(filename, line, column), literal_(literal) {}

// evaluate
std::string BooleanLiteralNode::evaluate() const {
    std::string type = "";
    if (checkMode) { type = " : bool"; }
    return std::string(literal_ ? ("true" + type) : ("false" + type));
}
