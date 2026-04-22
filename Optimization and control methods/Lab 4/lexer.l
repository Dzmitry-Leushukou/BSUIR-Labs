%{
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%option noyywrap
%option nounput
%option noinput

%%

\"n\"                return TOKEN_N;
\"m\"                return TOKEN_M;
\"c\"                return TOKEN_C;
\"A\"                return TOKEN_A;
\"b\"                return TOKEN_B;

[ \t\n\r]+          /* ignore whitespace */

-?[0-9]+(\.[0-9]+)? {
    yylval.num = atof(yytext);
    return NUMBER;
}

[,:]                return yytext[0];
[\[\]{}]            return yytext[0];

.                   { fprintf(stderr, "Unexpected character: %c\n", yytext[0]); }

%%
