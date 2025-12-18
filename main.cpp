#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "ast.h"
#include "codegen.h"

extern FILE* yyin;
extern int yyparse();
extern ProgramNode* root;
extern void reset_lexer();

void print_usage(const char* prog_name) {
    std::cerr << "Usage: " << prog_name << " <input.py> [-o output] [-c]\n";
    std::cerr << "  -o <file>   Specify output file (default: a.out)\n";
    std::cerr << "  -c          Generate object file instead of executable\n";
}

int compile_llvm_ir(const std::string& ir_code, const std::string& output_file, bool object_only) {
    // Write IR to temporary file
    std::string ir_file = output_file + ".ll";
    std::ofstream ir_out(ir_file);
    if (!ir_out) {
        std::cerr << "Error: Could not create IR file " << ir_file << std::endl;
        return 1;
    }
    ir_out << ir_code;
    ir_out.close();

    if (object_only) {
        // Compile to object file
        std::string obj_file = output_file;
        std::string cmd = "llc -filetype=obj " + ir_file + " -o " + obj_file;
        int result = system(cmd.c_str());
        if (result != 0) {
            std::cerr << "Error: llc failed\n";
            return 1;
        }
        std::cout << "Object file created: " << obj_file << std::endl;
    } else {
        // Compile to executable
        std::string obj_file = output_file + ".o";
        std::string cmd1 = "llc -filetype=obj " + ir_file + " -o " + obj_file;
        int result = system(cmd1.c_str());
        if (result != 0) {
            std::cerr << "Error: llc failed\n";
            return 1;
        }

        // Link to create executable
        std::string cmd2 = "gcc -no-pie " + obj_file + " -o " + output_file;
        result = system(cmd2.c_str());
        if (result != 0) {
            std::cerr << "Error: gcc linking failed\n";
            return 1;
        }

        // Clean up intermediate files
        remove(obj_file.c_str());

        std::cout << "Executable created: " << output_file << std::endl;
    }

    // Clean up IR file
    remove(ir_file.c_str());

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = "a.out";
    bool object_only = false;

    // Parse command-line arguments
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -o requires an argument\n";
                print_usage(argv[0]);
                return 1;
            }
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0) {
            object_only = true;
        } else {
            std::cerr << "Error: Unknown option " << argv[i] << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    // Open input file
    yyin = fopen(input_file.c_str(), "r");
    if (!yyin) {
        std::cerr << "Error: Could not open input file " << input_file << std::endl;
        return 1;
    }

    // Parse the input
    reset_lexer();
    int parse_result = yyparse();
    fclose(yyin);

    if (parse_result != 0) {
        std::cerr << "Error: Parsing failed\n";
        return 1;
    }

    if (!root) {
        std::cerr << "Error: No program parsed\n";
        return 1;
    }

    // Check for main function
    bool has_main = false;
    for (auto& func : root->functions) {
        if (func->name == "main") {
            has_main = true;
            break;
        }
    }

    if (!has_main) {
        std::cerr << "Error: Program must have a main() function\n";
        return 1;
    }

    // Generate LLVM IR
    CodeGenerator codegen;
    std::string ir_code = codegen.generate(root);

    // Compile to binary
    int result = compile_llvm_ir(ir_code, output_file, object_only);

    delete root;
    return result;
}
