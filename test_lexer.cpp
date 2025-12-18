#include <stdio.h>
#include <string>
#include "parser.tab.hpp"

extern FILE* yyin;
extern int yylex();
extern int yylineno;
extern void reset_lexer();
extern YYSTYPE yylval;

const char* token_name(int tok) {
    switch(tok) {
        case DEF: return "DEF";
        case RETURN: return "RETURN";
        case IF: return "IF";
        case ELIF: return "ELIF";
        case ELSE: return "ELSE";
        case WHILE: return "WHILE";
        case AND: return "AND";
        case OR: return "OR";
        case PRINT: return "PRINT";
        case NAME_VAR: return "NAME_VAR";
        case EQ: return "EQ";
        case NEQ: return "NEQ";
        case GT: return "GT";
        case LT: return "LT";
        case GTE: return "GTE";
        case LTE: return "LTE";
        case ASSIGN: return "ASSIGN";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case MULTIPLY: return "MULTIPLY";
        case DIVIDE: return "DIVIDE";
        case MODULO: return "MODULO";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case COLON: return "COLON";
        case COMMA: return "COMMA";
        case NEWLINE: return "NEWLINE";
        case INDENT: return "INDENT";
        case DEDENT: return "DEDENT";
        case IDENTIFIER: return "IDENTIFIER";
        case INTEGER: return "INTEGER";
        case STRING: return "STRING";
        default: return "UNKNOWN";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Could not open %s\n", argv[1]);
        return 1;
    }

    reset_lexer();
    int tok;
    while ((tok = yylex()) != 0) {
        printf("Line %d: %s", yylineno, token_name(tok));
        if (tok == IDENTIFIER || tok == STRING) {
            printf(" (%s)", yylval.str_val->c_str());
        } else if (tok == INTEGER) {
            printf(" (%d)", yylval.int_val);
        }
        printf("\n");
    }

    fclose(yyin);
    return 0;
}
