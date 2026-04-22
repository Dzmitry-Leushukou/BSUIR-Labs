%{
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%option noyywrap
%option nounput
%option noinput

%%

\"m\"                return TOKEN_M;
\"n\"                return TOKEN_N;
\"a\"                return TOKEN_A;
\"b\"                return TOKEN_B;
\"c\"                return TOKEN_C;

[ \t\n\r]+          /* ignore whitespace */

-?[0-9]+(\.[0-9]+)? {
    yylval.num = atof(yytext);
    return NUMBER;
}

[,:]                return yytext[0];
[\[\]{}]            return yytext[0];

.                   { fprintf(stderr, "Unexpected character: %c\n", yytext[0]); }

%%
