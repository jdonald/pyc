%code requires {
#include <string>
#include <vector>
#include <memory>
#include "ast.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <memory>
#include "ast.h"

extern int yylex();
extern int yylineno;
extern void reset_lexer();
void yyerror(const char *s);

ProgramNode* root = nullptr;
%}

%union {
    int int_val;
    std::string* str_val;
    ExprNode* expr;
    StmtNode* stmt;
    FunctionDefNode* func;
    ProgramNode* program;
    std::vector<std::unique_ptr<ExprNode>>* expr_list;
    std::vector<std::string>* str_list;
    std::vector<std::unique_ptr<StmtNode>>* stmt_list;
    std::vector<std::unique_ptr<FunctionDefNode>>* func_list;
}

%token <int_val> INTEGER
%token <str_val> IDENTIFIER STRING MAIN_STR
%token DEF RETURN IF ELIF ELSE WHILE AND OR PRINT NAME_VAR
%token EQ NEQ GT LT GTE LTE ASSIGN
%token PLUS MINUS MULTIPLY DIVIDE MODULO
%token LPAREN RPAREN COLON COMMA NEWLINE INDENT DEDENT

%type <expr> expression term factor primary comparison logical_and logical_or
%type <stmt> statement assignment if_statement while_statement return_statement expr_statement
%type <func> function_def
%type <program> program
%type <expr_list> arguments
%type <str_list> parameters
%type <stmt_list> statements
%type <func_list> function_list

%left OR
%left AND
%left EQ NEQ
%left LT GT LTE GTE
%left PLUS MINUS
%left MULTIPLY DIVIDE MODULO
%right UNEG UNOT

%%

program:
    function_list {
        root = new ProgramNode();
        if ($1) {
            for (auto& func : *$1) {
                root->functions.push_back(std::move(func));
            }
            delete $1;
        }
        $$ = root;
    }
    | function_list if_main_block {
        root = new ProgramNode();
        if ($1) {
            for (auto& func : *$1) {
                root->functions.push_back(std::move(func));
            }
            delete $1;
        }
        $$ = root;
    }
    ;

if_main_block:
    IF NAME_VAR EQ STRING COLON NEWLINE INDENT statements DEDENT {
        delete $4;
    }
    ;

function_list:
    function_def {
        $$ = new std::vector<std::unique_ptr<FunctionDefNode>>();
        $$->push_back(std::unique_ptr<FunctionDefNode>($1));
    }
    | function_list DEDENT function_def {
        $1->push_back(std::unique_ptr<FunctionDefNode>($3));
        $$ = $1;
    }
    | function_list function_def {
        $1->push_back(std::unique_ptr<FunctionDefNode>($2));
        $$ = $1;
    }
    | function_list NEWLINE {
        $$ = $1;
    }
    | function_list DEDENT {
        $$ = $1;
    }
    ;

function_def:
    DEF IDENTIFIER LPAREN parameters RPAREN COLON NEWLINE INDENT statements DEDENT {
        $$ = new FunctionDefNode(*$2, *$4, std::move(*$9));
        delete $2;
        delete $4;
        delete $9;
    }
    | DEF IDENTIFIER LPAREN RPAREN COLON NEWLINE INDENT statements DEDENT {
        $$ = new FunctionDefNode(*$2, std::vector<std::string>(), std::move(*$8));
        delete $2;
        delete $8;
    }
    ;

parameters:
    IDENTIFIER {
        $$ = new std::vector<std::string>();
        $$->push_back(*$1);
        delete $1;
    }
    | parameters COMMA IDENTIFIER {
        $1->push_back(*$3);
        delete $3;
        $$ = $1;
    }
    ;

statements:
    statement {
        $$ = new std::vector<std::unique_ptr<StmtNode>>();
        $$->push_back(std::unique_ptr<StmtNode>($1));
    }
    | statements statement {
        $1->push_back(std::unique_ptr<StmtNode>($2));
        $$ = $1;
    }
    | statements NEWLINE {
        $$ = $1;
    }
    ;

statement:
    assignment NEWLINE { $$ = $1; }
    | return_statement NEWLINE { $$ = $1; }
    | if_statement { $$ = $1; }
    | while_statement { $$ = $1; }
    | expr_statement NEWLINE { $$ = $1; }
    ;

assignment:
    IDENTIFIER ASSIGN expression {
        $$ = new AssignNode(*$1, std::unique_ptr<ExprNode>($3));
        delete $1;
    }
    ;

return_statement:
    RETURN expression {
        $$ = new ReturnNode(std::unique_ptr<ExprNode>($2));
    }
    | RETURN {
        $$ = new ReturnNode(std::unique_ptr<ExprNode>(new IntegerNode(0)));
    }
    ;

if_statement:
    IF expression COLON NEWLINE INDENT statements DEDENT {
        $$ = new IfNode(std::unique_ptr<ExprNode>($2), std::move(*$6), std::vector<std::unique_ptr<StmtNode>>());
        delete $6;
    }
    | IF expression COLON NEWLINE INDENT statements DEDENT ELSE COLON NEWLINE INDENT statements DEDENT {
        $$ = new IfNode(std::unique_ptr<ExprNode>($2), std::move(*$6), std::move(*$12));
        delete $6;
        delete $12;
    }
    ;

while_statement:
    WHILE expression COLON NEWLINE INDENT statements DEDENT {
        $$ = new WhileNode(std::unique_ptr<ExprNode>($2), std::move(*$6));
        delete $6;
    }
    ;

expr_statement:
    expression {
        $$ = new ExprStmtNode(std::unique_ptr<ExprNode>($1));
    }
    ;

expression:
    logical_or { $$ = $1; }
    ;

logical_or:
    logical_and { $$ = $1; }
    | logical_or OR logical_and {
        $$ = new BinaryOpNode(BinaryOp::OR, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    ;

logical_and:
    comparison { $$ = $1; }
    | logical_and AND comparison {
        $$ = new BinaryOpNode(BinaryOp::AND, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    ;

comparison:
    term { $$ = $1; }
    | term EQ term {
        $$ = new BinaryOpNode(BinaryOp::EQ, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    | term NEQ term {
        $$ = new BinaryOpNode(BinaryOp::NEQ, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    | term GT term {
        $$ = new BinaryOpNode(BinaryOp::GT, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    | term LT term {
        $$ = new BinaryOpNode(BinaryOp::LT, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    | term GTE term {
        $$ = new BinaryOpNode(BinaryOp::GTE, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    | term LTE term {
        $$ = new BinaryOpNode(BinaryOp::LTE, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    ;

term:
    factor { $$ = $1; }
    | term PLUS factor {
        $$ = new BinaryOpNode(BinaryOp::ADD, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    | term MINUS factor {
        $$ = new BinaryOpNode(BinaryOp::SUB, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    ;

factor:
    primary { $$ = $1; }
    | factor MULTIPLY primary {
        $$ = new BinaryOpNode(BinaryOp::MUL, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    | factor DIVIDE primary {
        $$ = new BinaryOpNode(BinaryOp::DIV, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    | factor MODULO primary {
        $$ = new BinaryOpNode(BinaryOp::MOD, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3));
    }
    ;

primary:
    INTEGER {
        $$ = new IntegerNode($1);
    }
    | IDENTIFIER {
        $$ = new IdentifierNode(*$1);
        delete $1;
    }
    | IDENTIFIER LPAREN arguments RPAREN {
        $$ = new CallNode(*$1, std::move(*$3));
        delete $1;
        delete $3;
    }
    | IDENTIFIER LPAREN RPAREN {
        $$ = new CallNode(*$1, std::vector<std::unique_ptr<ExprNode>>());
        delete $1;
    }
    | PRINT LPAREN expression RPAREN {
        std::vector<std::unique_ptr<ExprNode>> args;
        args.push_back(std::unique_ptr<ExprNode>($3));
        $$ = new CallNode("print", std::move(args));
    }
    | LPAREN expression RPAREN {
        $$ = $2;
    }
    | MINUS primary %prec UNEG {
        $$ = new UnaryOpNode(UnaryOp::NEG, std::unique_ptr<ExprNode>($2));
    }
    ;

arguments:
    expression {
        $$ = new std::vector<std::unique_ptr<ExprNode>>();
        $$->push_back(std::unique_ptr<ExprNode>($1));
    }
    | arguments COMMA expression {
        $1->push_back(std::unique_ptr<ExprNode>($3));
        $$ = $1;
    }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
}
