#include "codegen.h"
#include <iostream>

CodeGenerator::CodeGenerator() : temp_counter(0), label_counter(0) {}

std::string CodeGenerator::get_temp() {
    return "%t" + std::to_string(temp_counter++);
}

std::string CodeGenerator::get_label() {
    return "label" + std::to_string(label_counter++);
}

void CodeGenerator::declare_function(const std::string& name, const std::vector<std::string>& params) {
    functions[name] = params;
}

std::string CodeGenerator::codegen_expr(ExprNode* expr) {
    if (!expr) return "";

    switch (expr->type) {
        case NodeType::INTEGER: {
            IntegerNode* node = static_cast<IntegerNode*>(expr);
            return std::to_string(node->value);
        }

        case NodeType::IDENTIFIER: {
            IdentifierNode* node = static_cast<IdentifierNode*>(expr);
            if (variables.find(node->name) == variables.end()) {
                std::cerr << "Error: undefined variable " << node->name << std::endl;
                return "0";
            }
            std::string temp = get_temp();
            output << "  " << temp << " = load i32, i32* %"
                   << node->name << "\n";
            return temp;
        }

        case NodeType::BINARY_OP: {
            BinaryOpNode* node = static_cast<BinaryOpNode*>(expr);

            // Handle lazy evaluation for AND and OR
            if (node->op == BinaryOp::AND) {
                std::string left = codegen_expr(node->left.get());
                std::string left_bool = get_temp();
                output << "  " << left_bool << " = icmp ne i32 " << left << ", 0\n";

                std::string right_label = get_label();
                std::string end_label = get_label();
                std::string result_temp = get_temp();

                output << "  br i1 " << left_bool << ", label %" << right_label
                       << ", label %" << end_label << "\n";

                output << right_label << ":\n";
                std::string right = codegen_expr(node->right.get());
                std::string right_bool = get_temp();
                output << "  " << right_bool << " = icmp ne i32 " << right << ", 0\n";
                std::string right_int = get_temp();
                output << "  " << right_int << " = zext i1 " << right_bool << " to i32\n";
                output << "  br label %" << end_label << "\n";

                output << end_label << ":\n";
                output << "  " << result_temp << " = phi i32 [ 0, %"
                       << (label_counter - 3 >= 0 ? "label" + std::to_string(label_counter - 3) : current_function)
                       << " ], [ " << right_int << ", %" << right_label << " ]\n";

                return result_temp;
            }

            if (node->op == BinaryOp::OR) {
                std::string left = codegen_expr(node->left.get());
                std::string left_bool = get_temp();
                output << "  " << left_bool << " = icmp ne i32 " << left << ", 0\n";

                std::string right_label = get_label();
                std::string end_label = get_label();
                std::string result_temp = get_temp();

                output << "  br i1 " << left_bool << ", label %" << end_label
                       << ", label %" << right_label << "\n";

                output << right_label << ":\n";
                std::string right = codegen_expr(node->right.get());
                std::string right_bool = get_temp();
                output << "  " << right_bool << " = icmp ne i32 " << right << ", 0\n";
                std::string right_int = get_temp();
                output << "  " << right_int << " = zext i1 " << right_bool << " to i32\n";
                output << "  br label %" << end_label << "\n";

                output << end_label << ":\n";
                output << "  " << result_temp << " = phi i32 [ 1, %"
                       << (label_counter - 3 >= 0 ? "label" + std::to_string(label_counter - 3) : current_function)
                       << " ], [ " << right_int << ", %" << right_label << " ]\n";

                return result_temp;
            }

            // Regular binary operations
            std::string left = codegen_expr(node->left.get());
            std::string right = codegen_expr(node->right.get());
            std::string result = get_temp();

            switch (node->op) {
                case BinaryOp::ADD:
                    output << "  " << result << " = add i32 " << left << ", " << right << "\n";
                    break;
                case BinaryOp::SUB:
                    output << "  " << result << " = sub i32 " << left << ", " << right << "\n";
                    break;
                case BinaryOp::MUL:
                    output << "  " << result << " = mul i32 " << left << ", " << right << "\n";
                    break;
                case BinaryOp::DIV:
                    output << "  " << result << " = sdiv i32 " << left << ", " << right << "\n";
                    break;
                case BinaryOp::MOD:
                    output << "  " << result << " = srem i32 " << left << ", " << right << "\n";
                    break;
                case BinaryOp::EQ:
                    output << "  " << result << " = icmp eq i32 " << left << ", " << right << "\n";
                    {
                        std::string int_result = get_temp();
                        output << "  " << int_result << " = zext i1 " << result << " to i32\n";
                        return int_result;
                    }
                case BinaryOp::NEQ:
                    output << "  " << result << " = icmp ne i32 " << left << ", " << right << "\n";
                    {
                        std::string int_result = get_temp();
                        output << "  " << int_result << " = zext i1 " << result << " to i32\n";
                        return int_result;
                    }
                case BinaryOp::GT:
                    output << "  " << result << " = icmp sgt i32 " << left << ", " << right << "\n";
                    {
                        std::string int_result = get_temp();
                        output << "  " << int_result << " = zext i1 " << result << " to i32\n";
                        return int_result;
                    }
                case BinaryOp::LT:
                    output << "  " << result << " = icmp slt i32 " << left << ", " << right << "\n";
                    {
                        std::string int_result = get_temp();
                        output << "  " << int_result << " = zext i1 " << result << " to i32\n";
                        return int_result;
                    }
                case BinaryOp::GTE:
                    output << "  " << result << " = icmp sge i32 " << left << ", " << right << "\n";
                    {
                        std::string int_result = get_temp();
                        output << "  " << int_result << " = zext i1 " << result << " to i32\n";
                        return int_result;
                    }
                case BinaryOp::LTE:
                    output << "  " << result << " = icmp sle i32 " << left << ", " << right << "\n";
                    {
                        std::string int_result = get_temp();
                        output << "  " << int_result << " = zext i1 " << result << " to i32\n";
                        return int_result;
                    }
                default:
                    break;
            }
            return result;
        }

        case NodeType::UNARY_OP: {
            UnaryOpNode* node = static_cast<UnaryOpNode*>(expr);
            std::string operand = codegen_expr(node->operand.get());
            std::string result = get_temp();

            if (node->op == UnaryOp::NEG) {
                output << "  " << result << " = sub i32 0, " << operand << "\n";
            } else if (node->op == UnaryOp::NOT) {
                std::string bool_val = get_temp();
                output << "  " << bool_val << " = icmp eq i32 " << operand << ", 0\n";
                output << "  " << result << " = zext i1 " << bool_val << " to i32\n";
            }
            return result;
        }

        case NodeType::CALL: {
            CallNode* node = static_cast<CallNode*>(expr);

            // Handle print specially
            if (node->function_name == "print") {
                if (node->args.size() != 1) {
                    std::cerr << "Error: print takes exactly one argument\n";
                    return "0";
                }
                std::string arg = codegen_expr(node->args[0].get());
                std::string result = get_temp();
                output << "  " << result << " = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 " << arg << ")\n";
                return "0"; // print returns nothing meaningful
            }

            // Regular function call
            std::vector<std::string> arg_regs;
            for (auto& arg : node->args) {
                arg_regs.push_back(codegen_expr(arg.get()));
            }

            std::string result = get_temp();
            output << "  " << result << " = call i32 @" << node->function_name << "(";
            for (size_t i = 0; i < arg_regs.size(); i++) {
                if (i > 0) output << ", ";
                output << "i32 " << arg_regs[i];
            }
            output << ")\n";
            return result;
        }

        default:
            return "0";
    }
}

void CodeGenerator::codegen_stmt(StmtNode* stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case NodeType::ASSIGN: {
            AssignNode* node = static_cast<AssignNode*>(stmt);
            if (variables.find(node->var_name) == variables.end()) {
                // Allocate variable on first use
                variables[node->var_name] = 1;
                output << "  %" << node->var_name << " = alloca i32\n";
            }
            std::string value = codegen_expr(node->value.get());
            output << "  store i32 " << value << ", i32* %" << node->var_name << "\n";
            break;
        }

        case NodeType::RETURN_STMT: {
            ReturnNode* node = static_cast<ReturnNode*>(stmt);
            std::string value = codegen_expr(node->value.get());
            output << "  ret i32 " << value << "\n";
            break;
        }

        case NodeType::IF_STMT: {
            IfNode* node = static_cast<IfNode*>(stmt);
            std::string cond = codegen_expr(node->condition.get());
            std::string cond_bool = get_temp();
            output << "  " << cond_bool << " = icmp ne i32 " << cond << ", 0\n";

            std::string then_label = get_label();
            std::string else_label = get_label();
            std::string end_label = get_label();

            if (node->else_block.empty()) {
                output << "  br i1 " << cond_bool << ", label %" << then_label
                       << ", label %" << end_label << "\n";
            } else {
                output << "  br i1 " << cond_bool << ", label %" << then_label
                       << ", label %" << else_label << "\n";
            }

            output << then_label << ":\n";
            bool then_has_return = false;
            for (auto& s : node->then_block) {
                if (s->type == NodeType::RETURN_STMT) then_has_return = true;
                codegen_stmt(s.get());
            }
            if (!then_has_return) {
                output << "  br label %" << end_label << "\n";
            }

            bool else_has_return = false;
            if (!node->else_block.empty()) {
                output << else_label << ":\n";
                for (auto& s : node->else_block) {
                    if (s->type == NodeType::RETURN_STMT) else_has_return = true;
                    codegen_stmt(s.get());
                }
                if (!else_has_return) {
                    output << "  br label %" << end_label << "\n";
                }
            }

            // Only emit end label if at least one branch reaches it
            if (!then_has_return || !else_has_return) {
                output << end_label << ":\n";
            }
            break;
        }

        case NodeType::WHILE_STMT: {
            WhileNode* node = static_cast<WhileNode*>(stmt);
            std::string cond_label = get_label();
            std::string body_label = get_label();
            std::string end_label = get_label();

            output << "  br label %" << cond_label << "\n";
            output << cond_label << ":\n";

            std::string cond = codegen_expr(node->condition.get());
            std::string cond_bool = get_temp();
            output << "  " << cond_bool << " = icmp ne i32 " << cond << ", 0\n";
            output << "  br i1 " << cond_bool << ", label %" << body_label
                   << ", label %" << end_label << "\n";

            output << body_label << ":\n";
            for (auto& s : node->body) {
                codegen_stmt(s.get());
            }
            output << "  br label %" << cond_label << "\n";

            output << end_label << ":\n";
            break;
        }

        case NodeType::EXPR_STMT: {
            ExprStmtNode* node = static_cast<ExprStmtNode*>(stmt);
            codegen_expr(node->expr.get());
            break;
        }

        default:
            break;
    }
}

void CodeGenerator::codegen_function(FunctionDefNode* func) {
    variables.clear();
    current_function = func->name;

    // Declare function
    output << "define i32 @" << func->name << "(";
    for (size_t i = 0; i < func->params.size(); i++) {
        if (i > 0) output << ", ";
        output << "i32 %arg_" << func->params[i];
    }
    output << ") {\n";

    // Allocate and store parameters
    for (const auto& param : func->params) {
        variables[param] = 1;
        output << "  %" << param << " = alloca i32\n";
        output << "  store i32 %arg_" << param << ", i32* %" << param << "\n";
    }

    // Generate function body
    for (auto& stmt : func->body) {
        codegen_stmt(stmt.get());
    }

    output << "}\n\n";
}

std::string CodeGenerator::generate(ProgramNode* program) {
    std::ostringstream header;

    // Add printf declaration
    header << "@.str = private unnamed_addr constant [4 x i8] c\"%d\\0A\\00\", align 1\n";
    header << "declare i32 @printf(i8*, ...)\n\n";

    // First pass: collect function declarations for symbol table
    for (auto& func : program->functions) {
        declare_function(func->name, func->params);
    }

    // Generate function bodies
    for (auto& func : program->functions) {
        codegen_function(func.get());
    }

    return header.str() + output.str();
}
