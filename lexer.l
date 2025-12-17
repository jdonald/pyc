%{
#include <string>
#include <vector>
#include "parser.tab.hpp"

std::vector<int> indent_stack;
int pending_tokens[100];
int pending_count = 0;
int pending_index = 0;

#define YY_DECL int raw_yylex()
void process_indent(int spaces);
%}

%option noyywrap
%option yylineno

%x BOL

%%

<BOL>^[ \t]*\n     { /* skip blank lines */ }
<BOL>^[ \t]*#.*\n  { /* skip comment lines */ }

<BOL>^[ \t]*   {
                    /* Process indentation */
                    int spaces = 0;
                    for (int i = 0; i < yyleng; i++) {
                        spaces += (yytext[i] == '\t') ? 4 : 1;
                    }
                    process_indent(spaces);
                    BEGIN(INITIAL);
                    if (pending_count > 0) {
                        pending_index = 1;
                        return pending_tokens[0];
                    }
                }

\n              { BEGIN(BOL); return NEWLINE; }

#.*             { /* skip inline comments */ }

"def"           { return DEF; }
"return"        { return RETURN; }
"if"            { return IF; }
"elif"          { return ELIF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"and"           { return AND; }
"or"            { return OR; }
"print"         { return PRINT; }
"__name__"      { return NAME_VAR; }

"=="            { return EQ; }
"!="            { return NEQ; }
">="            { return GTE; }
"<="            { return LTE; }
">"             { return GT; }
"<"             { return LT; }
"="             { return ASSIGN; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULTIPLY; }
"/"             { return DIVIDE; }
"%"             { return MODULO; }

"("             { return LPAREN; }
")"             { return RPAREN; }
":"             { return COLON; }
","             { return COMMA; }

[a-zA-Z_][a-zA-Z0-9_]* {
                    yylval.str_val = new std::string(yytext);
                    return IDENTIFIER;
                }

[0-9]+          {
                    yylval.int_val = atoi(yytext);
                    return INTEGER;
                }

\"__main__\"    { yylval.str_val = new std::string("__main__"); return STRING; }
\'__main__\'    { yylval.str_val = new std::string("__main__"); return STRING; }

[ \t]+          { /* skip whitespace */ }

.               { fprintf(stderr, "Unexpected character: %c\n", yytext[0]); }

<<EOF>>         {
                    while (!indent_stack.empty()) {
                        indent_stack.pop_back();
                        pending_tokens[pending_count++] = DEDENT;
                    }
                    if (pending_count > pending_index) {
                        return pending_tokens[pending_index++];
                    }
                    return 0;
                }

%%

void process_indent(int spaces) {
    int current_level = indent_stack.empty() ? 0 : indent_stack.back();

    if (spaces > current_level) {
        indent_stack.push_back(spaces);
        pending_tokens[pending_count++] = INDENT;
    } else if (spaces < current_level) {
        while (!indent_stack.empty() && indent_stack.back() > spaces) {
            indent_stack.pop_back();
            pending_tokens[pending_count++] = DEDENT;
        }
        if (!indent_stack.empty() && indent_stack.back() != spaces) {
            fprintf(stderr, "Indentation error at line %d\n", yylineno);
        }
    }
}

int yylex() {
    if (pending_index < pending_count) {
        return pending_tokens[pending_index++];
    }
    pending_count = 0;
    pending_index = 0;
    return raw_yylex();
}

void reset_lexer() {
    indent_stack.clear();
    pending_count = 0;
    pending_index = 0;
    BEGIN(BOL);
}
