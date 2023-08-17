%skeleton "lalr1.cc" // -*- C++ -*-
%language "c++"
%require "3.7.5"
%locations

%defines

// Put parser inside a namespace
%define api.namespace {VSOP}

// Give the name of the parser class
%define api.parser.class {Parser}

// Force the token kind enum (used by the lexer) and the symbol kind enum
// (used by the parser) to use the same value for the tokens.
// (e.g. '+' will be represented by the same integer value in both enums.)
%define api.token.raw

// Tokens contain their type, value and location
// Also allow to use the make_TOKEN functions
%define api.token.constructor

// Allow to use C++ objects as semantic values
%define api.value.type variant

// Add some assertions.
%define parse.assert

// C++ code put inside header file
%code requires {
    #include <string>
    #include "ast.hpp"

    namespace VSOP
    {
        class Driver;
    }
}

// Add an argument to the parser constructor
%parse-param {VSOP::Driver &driver}

%code {
    #include "driver.hpp"

    using namespace std;

    vector<AST::ClassNode*> classes;
    vector<AST::FieldNode*> fields;
    vector<AST::MethodNode*> methods;
    vector<AST::FormalNode*> formals;
    vector<AST::ExprNode*> blocks;
    vector<AST::ExprNode*> exprs;
}

// Token and symbols definitions
%token
    LBRACE      "{"
    RBRACE      "}"
    LPAR        "("
    RPAR        ")"
    COLON       ":"
    SEMICOLON   ";"
    COMMA       ","
    PLUS        "+"
    MINUS       "-"
    TIMES       "*"
    DIV         "/"
    POW         "^"
    DOT         "."
    EQUAL       "="
    LOWER       "<"
    LOWER_EQUAL "<="
    ASSIGN      "<-"
;

// For some symbols, need to store a value
%token AND "and"
%token BOOL "bool"
%token CLASS "class"
%token DO "do"
%token ELSE "else"
%token EXTENDS "extends"
%token FALSE "false"
%token IF "if"
%token IN "in"
%token INT32 "int32"
%token ISNULL "isnull"
%token LET "let"
%token NEW "new"
%token NOT "not"
%token SELF "self"
%token STRING "string"
%token THEN "then"
%token TRUE "true"
%token UNIT "unit"
%token WHILE "while"

%token <std::string> TYPE_IDENTIFIER "type-identifier"
%token <std::string> OBJECT_IDENTIFIER "object-identifier"
%token <std::string> STRING_LITERAL "string-literal"

%token <int> INTEGER_LITERAL "integer-literal"

%token EOFERROR "eoferror"

// Precedence
%precedence "if" "then" "while" "let" "do" "in"
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%right "<-"
%left "and"
%right "not"
%nonassoc "<" "<=" "="
%left "+" "-"
%left "*" "/"
%right UMINUS "isnull"
%right "^"
%left "."

%type<AST::ProgramNode*> program
%type<AST::ClassNode*> class
%type<AST::ClassBodyNode*> class-body
%type<AST::FieldNode*> field
%type<AST::MethodNode*> method
%type<AST::TypeNode*> type
%type<AST::FormalsNode*> formals
%type<AST::FormalNode*> formal
%type<AST::BlockNode*> block
%type<AST::ExprNode*> expr
%type<AST::ArgsNode*> args
%%
// Grammar rules

%start program;
program:
    class-rec                   {
                                    std::string filename = *(@1.begin.filename);
                                    driver.res = new AST::ProgramNode(classes, filename, @1.begin.line, @1.begin.column);
                                    classes.clear();
                                };
class-rec:
    class                       {
                                    classes.push_back($1);
                                };
    | class class-rec           {
                                    classes.push_back($1);
                                };
class:
    "class" TYPE_IDENTIFIER class-body                                  {
                                                                            std::string* str = new std::string($2);
                                                                            std::string* str2 = new std::string("");
                                                                            std::string filename = *(@1.begin.filename);
                                                                            $$ = new AST::ClassNode(str, str2, $3, filename, @1.begin.line, @1.begin.column);
                                                                            delete str;
                                                                            delete str2;
                                                                        };
    | "class" TYPE_IDENTIFIER "extends" TYPE_IDENTIFIER class-body      {
                                                                            std::string* str1 = new std::string($2);
                                                                            std::string* str2 = new std::string($4);
                                                                            std::string filename = *(@1.begin.filename);
                                                                            $$ = new AST::ClassNode(str1, str2, $5, filename, @1.begin.line, @1.begin.column);
                                                                            delete str1;
                                                                            delete str2;
                                                                        };
class-body:
    LBRACE field-method RBRACE  {
                                    $$ = new AST::ClassBodyNode(fields, methods);
                                    fields.clear();
                                    methods.clear();
                                };
field-method:
    %empty                      {
                                    
                                };
    | field field-method        {
                                    fields.push_back($1);
                                };
    | method field-method       {
                                    methods.push_back($1);
                                };
field:
    TYPE_IDENTIFIER COLON type SEMICOLON                       {
                                                                    error(@1, "expected an object-identifier but type-identifier was given");
                                                                    YYERROR;
                                                               };
    | OBJECT_IDENTIFIER COLON type SEMICOLON                   {    
                                                                    std::string* str = new std::string($1);
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::FieldNode(str, $3, NULL, filename, @1.begin.line, @1.begin.column);
                                                                    delete str;
                                                               };
    | TYPE_IDENTIFIER COLON type ASSIGN expr SEMICOLON         {
                                                                    error(@1, "expected an object-identifier but type-identifier was given");
                                                                    YYERROR;
                                                               };
    | OBJECT_IDENTIFIER COLON type ASSIGN expr SEMICOLON       {    
                                                                    std::string* str = new std::string($1);
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::FieldNode(str, $3, $5, filename, @1.begin.line, @1.begin.column);
                                                                    delete str;
                                                               };
method:
    TYPE_IDENTIFIER LPAR formals RPAR COLON type block         {
                                                                    error(@1, "expected an object-identifier but type-identifier was given");
                                                                    YYERROR;
                                                               };
    | OBJECT_IDENTIFIER LPAR formals RPAR COLON type block     {    
                                                                    std::string* str = new std::string($1);
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::MethodNode(str, $3, $6, $7, filename, @1.begin.line, @1.begin.column);
                                                                    delete str;
                                                               };
type:
    OBJECT_IDENTIFIER           {
                                    error(@1, "expected a type-identifier but object-identifier was given");
                                    YYERROR;
                                };
    | TYPE_IDENTIFIER           {
                                    std::string* str = new std::string($1);
                                    std::string filename = *(@1.begin.filename);
                                    $$ = new AST::TypeNode(str, filename, @1.begin.line, @1.begin.column);
                                    delete str;
                                };
    | "int32"                   {
                                    std::string* str = new std::string("int32");
                                    std::string filename = *(@1.begin.filename);
                                    $$ = new AST::TypeNode(str, filename, @1.begin.line, @1.begin.column);
                                    delete str;
                                };
    | "bool"                    {
                                    std::string* str = new std::string("bool");
                                    std::string filename = *(@1.begin.filename);
                                    $$ = new AST::TypeNode(str, filename, @1.begin.line, @1.begin.column);
                                    delete str;
                                };
    | "string"                  {
                                    std::string* str = new std::string("string");
                                    std::string filename = *(@1.begin.filename);
                                    $$ = new AST::TypeNode(str, filename, @1.begin.line, @1.begin.column);
                                    delete str;
                                };
    | "unit"                    {
                                    std::string* str = new std::string("unit");
                                    std::string filename = *(@1.begin.filename);
                                    $$ = new AST::TypeNode(str, filename, @1.begin.line, @1.begin.column);
                                    delete str;
                                };
    | error                     {
                                    error(@1, "unexpected type");
                                    YYERROR;
                                };
formals:
    %empty                      {
                                    $$ = new AST::FormalsNode(formals);
                                    formals.clear();
                                    
                                };
    | formal                    {
                                    formals.push_back($1);
                                    $$ = new AST::FormalsNode(formals);
                                    formals.clear();
                                };
    | formal formals-rec        {
                                    formals.push_back($1);
                                    $$ = new AST::FormalsNode(formals);
                                    formals.clear();
                                };
formals-rec:
    COMMA formal                                                {
                                                                    formals.push_back($2);
                                                                };
    | COMMA formal formals-rec                                  {
                                                                    formals.push_back($2);
                                                                };
formal: OBJECT_IDENTIFIER COLON type                            {
                                                                    std::string* str = new std::string($1);
                                                                    $$ = new AST::FormalNode(str, $3);
                                                                    delete str;
                                                                };
block:

    LBRACE expr block-rec RBRACE                              {
                                                                    blocks.push_back($2);
                                                                    $$ = new AST::BlockNode(blocks);
                                                                    blocks.clear();
                                                                };
block-rec:
    %empty                      {
                                    
                                };
    | SEMICOLON expr block-rec  {
                                    blocks.push_back($2);
                                };
expr:
    "if" expr "then" expr %prec LOWER_THAN_ELSE                 {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::IfThenElseExprNode($2, $4, NULL, filename, @1.begin.line, @1.begin.column);
                                                                };
    /* | "if" expr "then" expr SEMICOLON "else" expr               {
                                                                    error(@5, "unexpected semicolon");
                                                                    YYERROR;
                                                                }; */
    | "if" expr "then" expr "else" expr                         {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::IfThenElseExprNode($2, $4, $6, filename, @1.begin.line, @1.begin.column);
                                                                };
    | "while" expr SEMICOLON "do" expr                          {
                                                                    error(@3, "unexpected semicolon");
                                                                    YYERROR;
                                                                };                                                                
    | "while" expr "do" expr                                    {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::WhileDoExprNode($2, $4, filename, @1.begin.line, @1.begin.column);
                                                                };
    | "let" TYPE_IDENTIFIER COLON type "in" expr                {
                                                                    error(@2, "expected an object-identifier but type-identifier was given");
                                                                    YYERROR;
                                                                };
    | "let" TYPE_IDENTIFIER COLON type ASSIGN expr "in" expr    {
                                                                    error(@2, "expected an object-identifier but type-identifier was given");
                                                                    YYERROR;
                                                                };                                                            
    | "let" OBJECT_IDENTIFIER COLON type "in" expr              {
                                                                    std::string* str = new std::string($2);
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::LetExprNode(str, $4, NULL, $6, filename, @1.begin.line, @1.begin.column);
                                                                    delete str;
                                                                };
    | "let" OBJECT_IDENTIFIER COLON type ASSIGN expr "in" expr  {
                                                                    std::string* str = new std::string($2);
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::LetExprNode(str, $4, $6, $8, filename, @1.begin.line, @1.begin.column);
                                                                    delete str;
                                                                };
    | TYPE_IDENTIFIER ASSIGN expr                               {
                                                                    error(@1, "expected an object-identifier but type-identifier was given");
                                                                    YYERROR;
                                                                };
    | OBJECT_IDENTIFIER ASSIGN expr                             {
                                                                    std::string* str = new std::string($1);
                                                                    std::string filename = *(@1.begin.filename);
                                                                    AST::VariableExprNode* variable = new AST::VariableExprNode(str, filename, @1.begin.line, @1.begin.column);
                                                                    std::string* str2 = new std::string($1);
                                                                    $$ = new AST::AssignExprNode(str2, variable, $3, filename, @1.begin.line, @1.begin.column);
                                                                    delete str;
                                                                    delete str2;
                                                                };
    | "not" expr                                                {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::NotNode($2, filename, @1.begin.line, @1.begin.column);
                                                                };
    | expr "and" expr                                           {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::AndNode($1, $3, filename, @1.begin.line, @1.begin.column);
                                                                };
    | expr EQUAL expr                                           {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::EqualNode($1, $3, filename, @1.begin.line, @1.begin.column);
                                                                };
    | expr LOWER expr                                           {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::LowerNode($1, $3, filename, @1.begin.line, @1.begin.column);
                                                                };
    | expr LOWER_EQUAL expr                                     {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::LowerEqualNode($1, $3, filename, @1.begin.line, @1.begin.column);
                                                                };
    | expr PLUS expr                                            {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::PlusNode($1, $3, filename, @1.begin.line, @1.begin.column);
                                                                };
    | expr MINUS expr                                           {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::MinusNode($1, $3, filename, @1.begin.line, @1.begin.column);
                                                                };
    | expr TIMES expr                                           {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::TimesNode($1, $3, filename, @1.begin.line, @1.begin.column);
                                                                };
    | expr DIV expr                                             {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::DivNode($1, $3, filename, @1.begin.line, @1.begin.column);
                                                                };
    | expr POW expr                                             {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::PowNode($1, $3, filename, @1.begin.line, @1.begin.column);
                                                                };
    | MINUS expr %prec UMINUS                                   {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::UnaryMinusNode($2, filename, @1.begin.line, @1.begin.column);
                                                                };
    | "isnull" expr                                             {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::IsNullNode($2, filename, @1.begin.line, @1.begin.column);
                                                                };
    | TYPE_IDENTIFIER LPAR args RPAR                            {
                                                                    error(@1, "expected an object-identifier but type-identifier was given");
                                                                    YYERROR;
                                                                };
    | expr DOT TYPE_IDENTIFIER LPAR args RPAR                   {
                                                                    error(@3, "expected an object-identifier but type-identifier was given");
                                                                    YYERROR;
                                                                };                                                            
    | OBJECT_IDENTIFIER LPAR args RPAR                          {
                                                                    std::string* str = new std::string($1);
                                                                    std::string* self = new std::string("self");
                                                                    std::string filename = *(@1.begin.filename);
                                                                    AST::ExprNode* objExpr = new AST::VariableExprNode(self, filename, @1.begin.line, @1.begin.column);
                                                                    $$ = new AST::CallExprNode(objExpr, str, $3, filename, @1.begin.line, @1.begin.column);
                                                                    delete self;
                                                                    delete str;

                                                                };
    | expr DOT OBJECT_IDENTIFIER LPAR args RPAR                 {
                                                                    std::string* str = new std::string($3);
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::CallExprNode($1, str, $5, filename, @1.begin.line, @1.begin.column);
                                                                    delete str;
                                                                };
    | "new" OBJECT_IDENTIFIER                                   {
                                                                    error(@2, "expected a type-identifier but object-identifier was given");
                                                                    YYERROR;
                                                                };
    | "new" TYPE_IDENTIFIER                                     {
                                                                    std::string* str = new std::string($2);
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::NewExprNode(str, filename, @1.begin.line, @1.begin.column);
                                                                    delete str;
                                                                };
    | TYPE_IDENTIFIER                                           {
                                                                    error(@1, "expected an object-identifier but type-identifier was given");
                                                                    YYERROR;
                                                                };
    | OBJECT_IDENTIFIER                                         {
                                                                    std::string* str = new std::string($1);
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::VariableExprNode(str, filename, @1.begin.line, @1.begin.column);
                                                                    delete str;
                                                                };
    | "self"                                                    {
                                                                    std::string* str = new std::string("self");
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::VariableExprNode(str, filename, @1.begin.line, @1.begin.column);
                                                                    delete str;
                                                                };
    | INTEGER_LITERAL           {
                                    std::string* str = new std::string(std::to_string($1));
                                    std::string filename = *(@1.begin.filename);
                                    $$ = new AST::IntegerLiteralNode(str, filename, @1.begin.line, @1.begin.column);
                                    delete str;
                                };
    | STRING_LITERAL            {
                                    std::string* str = new std::string("\"" + $1 +"\"");
                                    std::string filename = *(@1.begin.filename);
                                    $$ = new AST::LiteralNode(str, filename, @1.begin.line, @1.begin.column);
                                    delete str;
                                };
    | "true"                    {
                                    std::string filename = *(@1.begin.filename);
                                    $$ = new AST::BooleanLiteralNode(true, filename, @1.begin.line, @1.begin.column);
                                };
    | "false"                   {
                                    std::string filename = *(@1.begin.filename);
                                    $$ = new AST::BooleanLiteralNode(false, filename, @1.begin.line, @1.begin.column);
                                };
    | LPAR RPAR                                                 {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::UnitExprNode(filename, @1.begin.line, @1.begin.column);
                                                                };
    | LPAR expr RPAR                                            {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::ParExprNode($2, filename, @1.begin.line, @1.begin.column);
                                                                };
    | block                                                     {
                                                                    std::string filename = *(@1.begin.filename);
                                                                    $$ = new AST::BlockExprNode($1, filename, @1.begin.line, @1.begin.column);
                                                                };
args:
    %empty                      {
                                    $$ = new AST::ArgsNode(exprs);
                                    exprs.clear();
                                };
    | expr                      {
                                    exprs.push_back($1);
                                    $$ = new AST::ArgsNode(exprs);
                                    exprs.clear();
                                };
    | expr args-rec             {
                                    exprs.push_back($1);
                                    $$ = new AST::ArgsNode(exprs);
                                    exprs.clear();
                                };
args-rec:
    COMMA expr                  {
                                    exprs.push_back($2);
                                };
    | COMMA expr args-rec       {
                                    exprs.push_back($2);
                                };
%%
// User code
void VSOP::Parser::error(const location_type& l, const std::string& m) {
    const position &pos = l.begin;

    cerr << *(pos.filename) << ":"
         << pos.line << ":" 
         << pos.column << ": "
         << " syntax error: "
         << m
         << endl;

    exit(1);
}