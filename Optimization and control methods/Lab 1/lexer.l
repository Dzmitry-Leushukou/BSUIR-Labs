%{
#include "parser.tab.h"
#include <string.h>
%}

%option noyywrap
%option nounput
%option noinput

%%

\"size\"          { return TOKEN_SIZE; }
\"A\"             { return TOKEN_A; }
\"A_inv\"         { return TOKEN_A_INV; }
\"x\"             { return TOKEN_X; }
\"i\"             { return TOKEN_I; }

[ \t\n\r]+        { /* ignore whitespace */ }

-?[0-9]+(\.[0-9]+)? {
    yylval.num = atof(yytext);
    return NUMBER;
}

","               { return ','; }
":"               { return ':'; }
"["               { return '['; }
"]"               { return ']'; }
"{"               { return '{'; }
"}"               { return '}'; }

.                 { fprintf(stderr, "Unexpected character: %c\n", yytext[0]); }

%%