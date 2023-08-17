#ifndef _AST_HPP
#define _AST_HPP

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace AST {

    // Forward declaration
    class Visitor;

    class ClassNode;
    class ClassBodyNode;
    class FieldNode;
    class MethodNode;
    class TypeNode;
    class FormalsNode;
    class FormalNode;
    class BlockNode;
    class ExprNode;
    class ArgsNode;
    
    class ProgramNode;
    class IfThenElseExprNode;
    class WhileDoExprNode;
    class LetExprNode;
    class AssignExprNode;
    class BinaryExprNode;
    class AndNode;
    class EqualNode;
    class LowerNode;
    class LowerEqualNode;
    class PlusNode;
    class MinusNode;
    class TimesNode;
    class DivNode;
    class PowNode;
    class UnaryExprNode;
    class NewExprNode;
    class UnaryMinusNode;
    class NotNode;
    class IsNullNode;

    class VariableExprNode;
    class UnitExprNode;
    class ParExprNode;
    class BlockExprNode;
    class CallExprNode;
    class IntegerLiteralNode;
    class LiteralNode;
    class BooleanLiteralNode;
    class VariableExprNode;
    /*
    */
    enum class Operator {
        AND,
        EQUAL,
        LOWER,
        LOWER_EQUAL,
        PLUS,
        MINUS,
        TIMES,
        DIV,
        POW,
        NOT,
        ISNULL,
    };

    /*
    */
    enum class Type {
        UNIT,
        BOOL,
        INT32,
        STRING,
        CLASS
    };

    class Visitor {
        public:
            virtual void visit(ProgramNode* node) = 0;
            virtual void visit(TypeNode* node) = 0;
            virtual void visit(FormalNode* node) = 0;
            virtual void visit(FormalsNode* node) = 0;
            virtual void visit(ExprNode* node) = 0;
            virtual void visit(IfThenElseExprNode* node) = 0;
            virtual void visit(WhileDoExprNode* node) = 0;
            virtual void visit(LetExprNode* node) = 0;
            virtual void visit(AssignExprNode* node) = 0;
            virtual void visit(BinaryExprNode* node) = 0;

            virtual void visit(AndNode* node) = 0;
            virtual void visit(EqualNode* node) = 0;
            virtual void visit(LowerNode* node) = 0;
            virtual void visit(LowerEqualNode* node) = 0;
            virtual void visit(PlusNode* node) = 0;
            virtual void visit(MinusNode* node) = 0;
            virtual void visit(TimesNode* node) = 0;
            virtual void visit(DivNode* node) = 0;
            virtual void visit(PowNode* node) = 0;

            virtual void visit(UnaryExprNode* node) = 0;

            virtual void visit(UnaryMinusNode* node) = 0;
            virtual void visit(NotNode* node) = 0;
            virtual void visit(IsNullNode* node) = 0;

            virtual void visit(NewExprNode* node) = 0;
            virtual void visit(VariableExprNode* node) = 0;
            virtual void visit(UnitExprNode* node) = 0;
            virtual void visit(ParExprNode* node) = 0;
            virtual void visit(BlockNode* node) = 0;
            virtual void visit(BlockExprNode* node) = 0;
            virtual void visit(ArgsNode* node) = 0;
            virtual void visit(FieldNode* node) = 0;
            virtual void visit(MethodNode* node) = 0;
            virtual void visit(ClassBodyNode* node) = 0;
            virtual void visit(ClassNode* node) = 0;
            virtual void visit(CallExprNode* node) = 0;
            virtual void visit(IntegerLiteralNode* node) = 0;
            virtual void visit(LiteralNode* node) = 0;
            virtual void visit(BooleanLiteralNode* node) = 0;
    };

    /*
    */
    class AstNode {
        public:
            virtual ~AstNode() = default;
            virtual void accept(Visitor* visitor) = 0;
    };

    /*
    *   Program
    */
    class ProgramNode : public AstNode {
        public:
            ProgramNode(std::vector<ClassNode*> classes,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const;

            const std::vector<ClassNode*>& getClasses() const { return classes_; }

            std::string getFilename() const { return filename; }
            int getLine() const { return line; }
            int getColumn() const { return column; }

        protected:
            std::string filename;
            int line;
            int column;

        private:
            std::vector<ClassNode*> classes_;
    };

    /*
    *   Types
    */
    class TypeNode : public AstNode {
        public:
            TypeNode(std::string* type, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;           
            std::string evaluate() const;

            const std::string& getTypeStr() const { return typeStr_; }
            void setTypeStr(std::string t) {
                typeStr_ = t;
            }

            Type getType() { return type_; }
            void setType(Type t) {
                type_ = t;
            }

            std::string getFilename() const { return filename; }
            int getLine() const { return line; }
            int getColumn() const { return column; }

        protected:
            std::string filename;
            int line;
            int column;

        private:
            Type type_;
            std::string typeStr_;
    };

    class FormalNode : public AstNode {
        public:
            FormalNode(std::string* name, TypeNode* type);
            void accept(Visitor* visitor) override;
            std::string evaluate() const;

            const std::string& getName() const { return name_; }
            TypeNode* getType() { return type_; }

            ~FormalNode() { delete type_; }

        private:
            std::string name_;
            TypeNode* type_;
    };

    /*
    *   Formals
    */
    class FormalsNode : public AstNode {
        public:
            FormalsNode(std::vector<FormalNode*> formals);
            void accept(Visitor* visitor) override;
            std::string evaluate() const;

            const std::vector<FormalNode*>& getFormals() const { return formals_; }

            ~FormalsNode() {
                for (auto f : formals_) {
                    delete f;
                }
            }

        private:
            std::vector<FormalNode*> formals_;
    };


    /*
    *   Expressions
    */
    class ExprNode : public AstNode {
        public:
            ExprNode(const std::string& filename, int line, int column)
                : filename(filename), line(line), column(column) {}
            virtual ~ExprNode() = default;
            virtual std::string evaluate() const = 0;
            void accept(Visitor* visitor) override;

            std::string getFilename() const { return filename; }
            int getLine() const { return line; }
            int getColumn() const { return column; }

            Type getType() const { return type_; }
            void setType(Type t) {
                type_ = t;
            }

            std::string  getTypename() const { return typename_; }
            void setTypename(std::string t) {
                typename_ = t;
            }
        
        protected:
            std::string filename;
            int line;
            int column;

        private:
            Type type_;
            std::string typename_;
    };

    class IfThenElseExprNode : public ExprNode {
        public:
            IfThenElseExprNode(ExprNode* cond_expr, ExprNode* then_expr,
                ExprNode* else_expr, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            virtual ~IfThenElseExprNode() { delete cond_expr_;
                delete then_expr_; delete else_expr_; }
            ExprNode* getCondExpr() { return cond_expr_; }
            ExprNode* getThenExpr() { return then_expr_; }
            ExprNode* getElseExpr() { return else_expr_; }

        private:
            ExprNode* cond_expr_;
            ExprNode* then_expr_;
            ExprNode* else_expr_;
    };

    class WhileDoExprNode : public ExprNode {
        public:
            WhileDoExprNode(ExprNode* cond_expr, ExprNode* body_expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            virtual ~WhileDoExprNode() { delete cond_expr_; delete body_expr_; }
            ExprNode* getCondExpr() { return cond_expr_; }
            ExprNode* getBodyExpr() { return body_expr_; }

        private:
            ExprNode* cond_expr_;
            ExprNode* body_expr_;
    };

    class LetExprNode : public ExprNode {
        public:
            LetExprNode(std::string* name, TypeNode* type, ExprNode* init_expr,
                ExprNode* scope_expr, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            const std::string& getName() const { return name_; }
            TypeNode* getTypeNode() { return type_node_; }
            ExprNode* getInitExpr() { return init_expr_; }
            ExprNode* getScopeExpr() { return scope_expr_; }

            ~LetExprNode() { delete type_node_; delete init_expr_; delete scope_expr_;}

        private:
            std::string name_;
            TypeNode* type_node_;
            ExprNode* init_expr_;
            ExprNode* scope_expr_;
    };

    class AssignExprNode : public ExprNode {
        public:
            AssignExprNode(std::string* name_str, ExprNode* name, ExprNode* expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            const std::string& getNameStr() const { return name_str_; }
            ExprNode* getName() { return name_; }
            ExprNode* getExpr() { return expr_; }

            ~AssignExprNode() { delete expr_; }

        private:
            std::string name_str_;
            ExprNode* name_;
            ExprNode* expr_;
    };

    class BinaryExprNode : public ExprNode {
        public:
            BinaryExprNode(Operator op, ExprNode* left_expr,
                ExprNode* right_expr, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;
            
            virtual ~BinaryExprNode() { delete left_expr_; delete right_expr_; }
            ExprNode* getLeftExpr() { return left_expr_; }
            ExprNode* getRightExpr() { return right_expr_; }
            Operator getOp() const { return op_; }


        private:
            ExprNode* left_expr_;
            ExprNode* right_expr_;
            Operator op_;
    };

    // operators

    class AndNode : public BinaryExprNode {
        public:
            AndNode(ExprNode* left_expr, ExprNode* right_expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
    };

    class EqualNode : public BinaryExprNode {
        public:
            EqualNode(ExprNode* left_expr, ExprNode* right_expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
    };

    class LowerNode : public BinaryExprNode {
        public:
            LowerNode(ExprNode* left_expr, ExprNode* right_expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
    };

    class LowerEqualNode : public BinaryExprNode {
        public:
            LowerEqualNode(ExprNode* left_expr, ExprNode* right_expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
    };

    class PlusNode : public BinaryExprNode {
        public:
            PlusNode(ExprNode* left_expr, ExprNode* right_expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
    };

    class MinusNode : public BinaryExprNode {
        public:
            MinusNode(ExprNode* left_expr, ExprNode* right_expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
    };

    class TimesNode : public BinaryExprNode {
        public:
            TimesNode(ExprNode* left_expr, ExprNode* right_expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
    };

    class DivNode : public BinaryExprNode {
        public:
            DivNode(ExprNode* left_expr, ExprNode* right_expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
    };

    class PowNode : public BinaryExprNode {
        public:
            PowNode(ExprNode* left_expr, ExprNode* right_expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
    };

    class UnaryExprNode : public ExprNode {
        public:
            UnaryExprNode(Operator op, ExprNode* expr,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            virtual ~UnaryExprNode() { delete expr_; }
            ExprNode* getNode() { return expr_; }
            Operator getOperator() const { return op_; }

        protected:
            ExprNode* expr_;
            Operator op_;
    };

    // operators

    class UnaryMinusNode : public UnaryExprNode {
        public:
            UnaryMinusNode(ExprNode* expr, const std::string& filename,
                int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;
    };

    class NotNode : public UnaryExprNode {
        public:
            NotNode(ExprNode* expr, const std::string& filename,
                int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;
    };

    class IsNullNode : public UnaryExprNode {
        public:
            IsNullNode(ExprNode* expr, const std::string& filename,
                int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;
    };

    /*
    *   New
    */
    class NewExprNode : public ExprNode {
        public:
            NewExprNode(std::string* type_name, const std::string& filename,
                int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            const std::string& getTypeName() const { return type_name_; }

            ~NewExprNode() {}

        private:
            std::string type_name_;
    };

    /*
    *   Variable names
    */
    class VariableExprNode : public ExprNode {
        public:
            VariableExprNode(std::string* variable_name,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            std::string& getVariableName() { return variable_name_; }

            ~VariableExprNode() {}

        private:
            std::string variable_name_;
    };
    
    /*
    *   Unit
    */
    class UnitExprNode : public ExprNode {
        public:
            UnitExprNode(const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;
    };
    
    class ParExprNode : public ExprNode {
        public:
            ParExprNode(ExprNode* expr, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            virtual ~ParExprNode() { delete expr_; }
            ExprNode* getExpr() { return expr_; }

        private:
            ExprNode* expr_;
    };
    /*
    *   Blocks
    */
    class BlockNode : public AstNode {
        public:
            BlockNode(std::vector<ExprNode*> expr_list);
            void accept(Visitor* visitor) override;
            std::string evaluate() const;
            
            std::vector<ExprNode*>& getExpr() { return expr_list_; }

            ~BlockNode() {
                for (auto e : expr_list_) {
                    delete e;
                }
            }

            Type getType() const { return type_; }
            void setType(Type t) {
                type_ = t;
            }

            std::string  getTypename() const { return typename_; }
            void setTypename(std::string t) {
                typename_ = t;
            }

        private:
            Type type_;
            std::string typename_;
            std::vector<ExprNode*> expr_list_;
    };

    /*
    *   BlockExpr
    */
    class BlockExprNode : public ExprNode {
        public:
            BlockExprNode(BlockNode* block, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            BlockNode* getBlock() { return block_; }

            ~BlockExprNode() { delete block_; }

        private:
            BlockNode* block_;
    };

    class ArgsNode : public AstNode {
        public:
            ArgsNode(std::vector<ExprNode*> exprs);
            void accept(Visitor* visitor) override;
            std::string evaluate() const;
            
            std::vector<ExprNode*>& getExprs() { return exprs_; }

            ~ArgsNode() {
                for (auto e : exprs_) {
                    delete e;
                }
            }

        private:
        std::vector<ExprNode*> exprs_;
    };
    
    /*
    *   Field
    */
    class FieldNode : public AstNode {
        public:
            FieldNode(std::string* name, TypeNode* type,
                ExprNode* init_expr, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const;

            const std::string& getName() const { return name_; }
            TypeNode* getType() { return type_; }
            ExprNode* getInitExpr() { return init_expr_; }

            ~FieldNode() { delete type_; delete init_expr_; }

            std::string getFilename() const { return filename; }
            int getLine() const { return line; }
            int getColumn() const { return column; }

        protected:
            std::string filename;
            int line;
            int column;

        private:
            std::string name_;
            TypeNode* type_;
            ExprNode* init_expr_;
    };

    /*
    *   Method
    */
    class MethodNode : public AstNode {
        public:
            MethodNode(std::string* name, FormalsNode* formals,
                TypeNode* ret_type, BlockNode* block, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const;

            const std::string& getName() const { return name_; }
            FormalsNode* getFormals() { return formals_; }
            TypeNode* getRetType() { return ret_type_; }
            BlockNode* getBlock() { return block_; }

            ~MethodNode() {
                delete formals_;
                delete ret_type_;
                delete block_;
            }

            std::string getFilename() const { return filename; }
            int getLine() const { return line; }
            int getColumn() const { return column; }

        protected:
            std::string filename;
            int line;
            int column;

        private:
            std::string name_;
            FormalsNode* formals_;
            TypeNode* ret_type_;
            BlockNode* block_;
    };

    class ClassBodyNode : public AstNode {
        public:
            ClassBodyNode(std::vector<FieldNode*> fields, std::vector<MethodNode*> methods);
            void accept(Visitor* visitor) override;
            std::string evaluate() const;

            const std::vector<FieldNode*>& getFields() const { return fields_; }
            const std::vector<MethodNode*>& getMethods() const { return methods_; }

            ~ClassBodyNode() {
                for (auto f : fields_) {
                    delete f;
                }
                for (auto m : methods_) {
                    delete m;
                }
            }

        private:
            std::vector<FieldNode*> fields_;
            std::vector<MethodNode*> methods_;
    };

    /*
    *   Class
    */
    class ClassNode : public AstNode {
        public:
            ClassNode(std::string* name, std::string* parent,
                ClassBodyNode* class_body, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const;

            const std::string& getName() const { return name_; }
            std::string& getParent() { return parent_; }
            void setParent(std::string p) {
                parent_ = p;
            }
            ClassBodyNode* getClassBody() { return class_body_; }

            ~ClassNode() { delete class_body_; }

            Type getType() { return type_; }
            void setType(Type t) {
                type_ = t;
            }

            std::string getFilename() const { return filename; }
            int getLine() const { return line; }
            int getColumn() const { return column; }

        protected:
            std::string filename;
            int line;
            int column;

        private:
            Type type_;
            std::string name_;
            std::string parent_;
            ClassBodyNode* class_body_;
    };

    /*
    *   Call
    */
    class CallExprNode : public ExprNode {
        public:
            CallExprNode(ExprNode* obj_expr, std::string* method_name,
                ArgsNode* expr_list, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            const std::string& getMethodName() const { return method_name_; }
            ExprNode* getObjExpr() { return obj_expr_; }
            ArgsNode* getExprList() { return expr_list_; }

            ~CallExprNode() { delete obj_expr_; delete expr_list_;}

        private:
            std::string method_name_;
            ExprNode* obj_expr_;
            ArgsNode* expr_list_;
    };

    class IntegerLiteralNode : public ExprNode {
        public:
            IntegerLiteralNode(std::string* integerliteral,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            const std::string& getIntegerLiteral() const { return integerliteral_; }

        private:
            std::string integerliteral_;
    };

    class LiteralNode : public ExprNode {
        public:
            LiteralNode(std::string* literal,
                const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;

            const std::string& getLiteral() const { return literal_; }

        private:
            std::string literal_;
    };

    class BooleanLiteralNode : public ExprNode {
        public:
            BooleanLiteralNode(bool literal, const std::string& filename, int line, int column);
            void accept(Visitor* visitor) override;
            std::string evaluate() const override;
            
            bool getLiteral() const { return literal_; }

        private:
            bool literal_;
    };
}
#endif
