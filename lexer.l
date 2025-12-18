%{
#include <string>
#include <vector>
#include "parser.tab.hpp"

std::vector<int> indent_stack;
std::vector<int> pending_tokens;
size_t pending_idx = 0;

void process_indent(int spaces);

#define YY_DECL int yylex_orig()
int yylex();
%}

%option noyywrap
%option yylineno

%x INDENT_CHECK

%%

<INDENT_CHECK>[ \t]*\n      { /* skip blank lines */ }
<INDENT_CHECK>[ \t]*#[^\n]*\n { /* skip comment lines */ }

<INDENT_CHECK>[ \t]+        {
                                /* Line with indentation */
                                int spaces = 0;
                                for (int i = 0; i < yyleng; i++) {
                                    spaces += (yytext[i] == '\t') ? 4 : 1;
                                }
                                process_indent(spaces);
                                BEGIN(INITIAL);
                                if (!pending_tokens.empty()) {
                                    pending_idx = 1;
                                    return pending_tokens[0];
                                }
                            }

<INDENT_CHECK>.             {
                                /* Line with no indentation - put char back */
                                yyless(0);
                                process_indent(0);
                                BEGIN(INITIAL);
                                if (!pending_tokens.empty()) {
                                    pending_idx = 1;
                                    return pending_tokens[0];
                                }
                            }

<INITIAL>\n                 { BEGIN(INDENT_CHECK); return NEWLINE; }

<INITIAL>#[^\n]*            { /* skip comments */ }

<INITIAL>"def"              { return DEF; }
<INITIAL>"return"           { return RETURN; }
<INITIAL>"if"               { return IF; }
<INITIAL>"elif"             { return ELIF; }
<INITIAL>"else"             { return ELSE; }
<INITIAL>"while"            { return WHILE; }
<INITIAL>"and"              { return AND; }
<INITIAL>"or"               { return OR; }
<INITIAL>"print"            { return PRINT; }
<INITIAL>"__name__"         { return NAME_VAR; }

<INITIAL>"=="               { return EQ; }
<INITIAL>"!="               { return NEQ; }
<INITIAL>">="               { return GTE; }
<INITIAL>"<="               { return LTE; }
<INITIAL>">"                { return GT; }
<INITIAL>"<"                { return LT; }
<INITIAL>"="                { return ASSIGN; }
<INITIAL>"+"                { return PLUS; }
<INITIAL>"-"                { return MINUS; }
<INITIAL>"*"                { return MULTIPLY; }
<INITIAL>"/"                { return DIVIDE; }
<INITIAL>"%"                { return MODULO; }

<INITIAL>"("                { return LPAREN; }
<INITIAL>")"                { return RPAREN; }
<INITIAL>":"                { return COLON; }
<INITIAL>","                { return COMMA; }

<INITIAL>[a-zA-Z_][a-zA-Z0-9_]* {
                                yylval.str_val = new std::string(yytext);
                                return IDENTIFIER;
                            }

<INITIAL>[0-9]+             {
                                yylval.int_val = atoi(yytext);
                                return INTEGER;
                            }

<INITIAL>\"__main__\"       { yylval.str_val = new std::string("__main__"); return STRING; }
<INITIAL>\'__main__\'       { yylval.str_val = new std::string("__main__"); return STRING; }

<INITIAL>[ \t]+             { /* skip whitespace */ }

<INITIAL,INDENT_CHECK>.     { fprintf(stderr, "Unexpected character: '%c' at line %d\n", yytext[0], yylineno); }

<<EOF>>                     {
                                while (!indent_stack.empty()) {
                                    indent_stack.pop_back();
                                    pending_tokens.push_back(DEDENT);
                                }
                                if (pending_idx < pending_tokens.size()) {
                                    return pending_tokens[pending_idx++];
                                }
                                return 0;
                            }

%%

void process_indent(int spaces) {
    int current_level = indent_stack.empty() ? 0 : indent_stack.back();

    if (spaces > current_level) {
        indent_stack.push_back(spaces);
        pending_tokens.push_back(INDENT);
    } else if (spaces < current_level) {
        while (!indent_stack.empty() && indent_stack.back() > spaces) {
            indent_stack.pop_back();
            pending_tokens.push_back(DEDENT);
        }
        if (!indent_stack.empty() && indent_stack.back() != spaces) {
            fprintf(stderr, "Indentation error at line %d\n", yylineno);
        }
    }
}

int yylex() {
    if (pending_idx < pending_tokens.size()) {
        return pending_tokens[pending_idx++];
    }
    pending_tokens.clear();
    pending_idx = 0;
    return yylex_orig();
}

void reset_lexer() {
    indent_stack.clear();
    pending_tokens.clear();
    pending_idx = 0;
    BEGIN(INDENT_CHECK);
}
