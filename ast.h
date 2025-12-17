#ifndef AST_H
#define AST_H

#include <vector>
#include <string>
#include <memory>

enum class NodeType {
    PROGRAM,
    FUNCTION_DEF,
    BLOCK,
    RETURN_STMT,
    IF_STMT,
    WHILE_STMT,
    EXPR_STMT,
    ASSIGN,
    BINARY_OP,
    UNARY_OP,
    CALL,
    IDENTIFIER,
    INTEGER
};

enum class BinaryOp {
    ADD, SUB, MUL, DIV, MOD,
    EQ, GT, LT, GTE, LTE, NEQ,
    AND, OR
};

enum class UnaryOp {
    NEG, NOT
};

class ASTNode {
public:
    NodeType type;
    virtual ~ASTNode() = default;
protected:
    ASTNode(NodeType t) : type(t) {}
};

class ExprNode : public ASTNode {
public:
    ExprNode(NodeType t) : ASTNode(t) {}
};

class IntegerNode : public ExprNode {
public:
    int value;
    IntegerNode(int val) : ExprNode(NodeType::INTEGER), value(val) {}
};

class IdentifierNode : public ExprNode {
public:
    std::string name;
    IdentifierNode(const std::string& n) : ExprNode(NodeType::IDENTIFIER), name(n) {}
};

class BinaryOpNode : public ExprNode {
public:
    BinaryOp op;
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
    BinaryOpNode(BinaryOp o, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r)
        : ExprNode(NodeType::BINARY_OP), op(o), left(std::move(l)), right(std::move(r)) {}
};

class UnaryOpNode : public ExprNode {
public:
    UnaryOp op;
    std::unique_ptr<ExprNode> operand;
    UnaryOpNode(UnaryOp o, std::unique_ptr<ExprNode> operand)
        : ExprNode(NodeType::UNARY_OP), op(o), operand(std::move(operand)) {}
};

class CallNode : public ExprNode {
public:
    std::string function_name;
    std::vector<std::unique_ptr<ExprNode>> args;
    CallNode(const std::string& name, std::vector<std::unique_ptr<ExprNode>> a)
        : ExprNode(NodeType::CALL), function_name(name), args(std::move(a)) {}
};

class StmtNode : public ASTNode {
public:
    StmtNode(NodeType t) : ASTNode(t) {}
};

class AssignNode : public StmtNode {
public:
    std::string var_name;
    std::unique_ptr<ExprNode> value;
    AssignNode(const std::string& name, std::unique_ptr<ExprNode> val)
        : StmtNode(NodeType::ASSIGN), var_name(name), value(std::move(val)) {}
};

class ExprStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> expr;
    ExprStmtNode(std::unique_ptr<ExprNode> e)
        : StmtNode(NodeType::EXPR_STMT), expr(std::move(e)) {}
};

class ReturnNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> value;
    ReturnNode(std::unique_ptr<ExprNode> val)
        : StmtNode(NodeType::RETURN_STMT), value(std::move(val)) {}
};

class IfNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::vector<std::unique_ptr<StmtNode>> then_block;
    std::vector<std::unique_ptr<StmtNode>> else_block;
    IfNode(std::unique_ptr<ExprNode> cond,
           std::vector<std::unique_ptr<StmtNode>> then_b,
           std::vector<std::unique_ptr<StmtNode>> else_b)
        : StmtNode(NodeType::IF_STMT), condition(std::move(cond)),
          then_block(std::move(then_b)), else_block(std::move(else_b)) {}
};

class WhileNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::vector<std::unique_ptr<StmtNode>> body;
    WhileNode(std::unique_ptr<ExprNode> cond, std::vector<std::unique_ptr<StmtNode>> b)
        : StmtNode(NodeType::WHILE_STMT), condition(std::move(cond)), body(std::move(b)) {}
};

class FunctionDefNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<StmtNode>> body;
    FunctionDefNode(const std::string& n, std::vector<std::string> p,
                    std::vector<std::unique_ptr<StmtNode>> b)
        : ASTNode(NodeType::FUNCTION_DEF), name(n), params(std::move(p)), body(std::move(b)) {}
};

class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<FunctionDefNode>> functions;
    ProgramNode() : ASTNode(NodeType::PROGRAM) {}
};

#endif // AST_H
