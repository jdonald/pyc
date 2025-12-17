#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include "ast.h"

class CodeGenerator {
private:
    std::ostringstream output;
    std::map<std::string, int> variables; // Maps variable names to their register numbers
    std::map<std::string, std::vector<std::string>> functions; // Maps function names to parameter lists
    int temp_counter;
    int label_counter;
    std::string current_function;

    std::string get_temp();
    std::string get_label();
    std::string codegen_expr(ExprNode* expr);
    void codegen_stmt(StmtNode* stmt);
    void codegen_function(FunctionDefNode* func);

public:
    CodeGenerator();
    std::string generate(ProgramNode* program);
    void declare_function(const std::string& name, const std::vector<std::string>& params);
};

#endif // CODEGEN_H
